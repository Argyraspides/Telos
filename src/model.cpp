#include "model.h"
#include "engine_math.h"
#include "application_params.h"
#include <chrono>
#include <iostream>
#include <algorithm>

Model::Model()
{
    this->m_shapeType = SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX;

    pthread_mutex_init(&this->shapeListMutex, nullptr);
    pthread_mutex_init(&this->PCSCVXShapeListMutex, nullptr);
}

Model::~Model()
{
    pthread_mutex_destroy(&this->shapeListMutex);
    pthread_mutex_destroy(&this->PCSCVXShapeListMutex);
}

// The engine can only handle shapes of one particular data structure at a time. Here we resolve what
// kind of shapes we are dealing with and then begin the respective loop that handles those particular
// types of shapes. By default the shape type is of type point cloud.
void Model::run()
{
    this->m_isRunning = true;
    this->m_isPaused = false;
    if (this->m_shapeType == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        // Control the loop to execute at the desired frequency
        const std::chrono::milliseconds frameDuration((long)(m_timeStep * 1000));
        auto startTime = std::chrono::high_resolution_clock::now();

        while (m_isRunning)
        {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

            if (elapsed >= frameDuration && !m_isPaused)
            {
                // Update all shapes including their positions, resolve their collisions, etc
                updatePCSL();
                m_time += m_timeStep;
                // std::cout << "CURRENT TIME: " << time << "\n";
                startTime = std::chrono::high_resolution_clock::now();
            }
        }
    }

    else if (this->m_shapeType == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_ARB)
    {
        while (m_isRunning)
        {
        }
    }
}
// Update Point Cloud Shape List
void Model::updatePCSL()
{
    // Translate the shapes
    // Calling "getPCSCVXShapeList()" locks access to the shape list.
    int size = this->getPCSCVXShapeList().size();

    for (int i = 0; i < size; i++)
    {
        this->m_PCSCVX_shapeList[i]->updateShape(m_timeStep, TIMEDIR::FORWARD);
    }

    for (int i = 0; i < size - 1; i++)
    {
        for (int j = i + 1; j < size; j++)
        {
            CollisionInfo_PCSCVX c = isContactPCSCVX_CL(*this->m_PCSCVX_shapeList[i], *this->m_PCSCVX_shapeList[j]);
            if (c.hasCollided)
            {
                resolveCollisionOverlapPCSCVX(c);
                resolveCollisionPCSCVX(c);
            }
        }
    }

    for (int i = 0; i < size; i++)
    {
        WallCollisionInfo_PCSCVX wci = isContactWallLinear(*this->m_PCSCVX_shapeList[i]);
        if (wci.collided)
        {
            resolveOverlapCollisionPCSCVX_Wall_Linear(wci);
            resolveCollisionPCSCVX_Wall(wci);
        }
    }

    // Unlock access to the shape list now that we are done
    pthread_mutex_unlock(&PCSCVXShapeListMutex);
}

// MAKE SURE TO CALL pthread_mutex_unlock() ONCE YOU ARE DONE ACCESSING THE LIST
std::vector<std::shared_ptr<Shape>> &Model::getShapeList()
{
    pthread_mutex_lock(&shapeListMutex);
    return this->m_shapeList;
}
// MAKE SURE TO CALL pthread_mutex_unlock() ONCE YOU ARE DONE ACCESSING THE LIST
std::vector<std::shared_ptr<PointCloudShape_Cvx>> &Model::getPCSCVXShapeList()
{
    pthread_mutex_lock(&PCSCVXShapeListMutex);
    return this->m_PCSCVX_shapeList;
}

// Uses the SAT (Separation Axis Theorem) to detect collision
bool Model::isContactPCSCVX_SAT(PointCloudShape_Cvx &s1, PointCloudShape_Cvx &s2)
{
    PointCloudShape_Cvx *_s1 = &s1;
    PointCloudShape_Cvx *_s2 = &s2;

    for (int s = 0; s < 2; s++)
    {
        // On the second iteration, we compute shape 2's projection lines and test for collision
        if (s == 1)
        {
            _s1 = &s2;
            _s2 = &s1;
        }

        for (int v = 0; v < _s1->m_points.size(); v++)
        {
            // Gradient, or vector, of the line that is made from one shape's vertex to the next
            Point grad = _s1->m_points[(v + 1) % _s1->m_points.size()] - _s1->m_points[v];
            // Projection axis is the normal of the gradient vector
            Point projectionAxis = Math::getNormal2D(grad);

            double min1 = std::numeric_limits<double>::infinity(),
                   max1 = -min1;

            // Obtain minimum and maximum points of projection for each shape

            for (int p = 0; p < _s1->m_points.size(); p++)
            {
                double dotProd = Math::dotProd(_s1->m_points[p], projectionAxis);
                min1 = std::min(min1, dotProd);
                max1 = std::max(max1, dotProd);
            }

            double min2 = std::numeric_limits<double>::infinity(),
                   max2 = -min2;

            for (int p = 0; p < _s2->m_points.size(); p++)
            {
                double dotProd = Math::dotProd(_s2->m_points[p], projectionAxis);
                min2 = std::min(min2, dotProd);
                max2 = std::max(max2, dotProd);
            }

            if (max1 < min2 || max2 < min1)
            {
                return false;
            }
        }
    }
    return true;
}

// Uses simple line intersections to detect collision, collision point, collision surface, and penetration depth.
// Skibidi toilet. Time complexity O(n^2)
CollisionInfo_PCSCVX Model::isContactPCSCVX_CL(PointCloudShape_Cvx &s1, PointCloudShape_Cvx &s2)
{

    PointCloudShape_Cvx *_s1 = &s1;
    PointCloudShape_Cvx *_s2 = &s2;

    // X and Y bounds of the edge of the other polygon we are comparing against
    double edgeLineBoundsX[2] = {0, 0};
    double edgeLineBoundsY[2] = {0, 0};
    // X and Y bounds of the line drawn from the center to a vertex of the polygon we are comparing with
    double centerLineBoundsX[2] = {0, 0};
    double centerLineBoundsY[2] = {0, 0};

    for (int s = 0; s < 2; s++)
    {

        // Need to compare polygon 1's center lines against polygon 2's edge lines and vice versa
        if (s == 1)
        {
            _s1 = &s2;
            _s2 = &s1;
        }

        for (int i = 0; i < _s1->m_points.size(); i++)
        {
            // Center line is the line drawn from the center of the polygon to one of its vertices
            Line centerLine(_s1->m_center, _s1->m_points[i]);

            // Its X and Y bounds are simply the start and end of this line
            centerLineBoundsY[0] = std::min(_s1->m_center.y, _s1->m_points[i].y);
            centerLineBoundsY[1] = std::max(_s1->m_center.y, _s1->m_points[i].y);

            centerLineBoundsX[0] = std::min(_s1->m_center.x, _s1->m_points[i].x);
            centerLineBoundsX[1] = std::max(_s1->m_center.x, _s1->m_points[i].x);

            for (int j = 0; j < _s2->m_points.size(); j++)
            {
                int wrap = (j + 1) % _s2->m_points.size();
                // The bounds of the edge line is simply the beginning and end of the line that makes this edge of the polygon
                edgeLineBoundsY[0] = std::min(_s2->m_points[j].y, _s2->m_points[wrap].y);
                edgeLineBoundsY[1] = std::max(_s2->m_points[j].y, _s2->m_points[wrap].y);

                edgeLineBoundsX[0] = std::min(_s2->m_points[j].x, _s2->m_points[wrap].x);
                edgeLineBoundsX[1] = std::max(_s2->m_points[j].x, _s2->m_points[wrap].x);

                // equation of the line that forms the polygon edge
                Line edgeLine(_s2->m_points[j], _s2->m_points[wrap]);

                // Intersection point of the line created by connecting the center of polygon _s1 to one of its vertices, and the line
                // that defines the current edge of polygon _s2
                Point intersection = Math::intersectionPt(centerLine, edgeLine);

                // If this intersection is within the bounds of both lines, there is a collision
                bool inCenterBoundsX = (intersection.x >= centerLineBoundsX[0] && intersection.x <= centerLineBoundsX[1]);
                bool inCenterBoundsY = (intersection.y >= centerLineBoundsY[0] && intersection.y <= centerLineBoundsY[1]);
                bool inEdgeBoundsX = (intersection.x >= edgeLineBoundsX[0] && intersection.x <= edgeLineBoundsX[1]);
                bool inEdgeBoundsY = (intersection.y >= edgeLineBoundsY[0] && intersection.y <= edgeLineBoundsY[1]);

                if (inCenterBoundsX && inCenterBoundsY && inEdgeBoundsX && inEdgeBoundsY)
                {
                    Point collisionSurfaceNormal = Math::getNormal2D((_s2->m_points[j] - _s2->m_points[wrap]));
                    collisionSurfaceNormal.normalize();

                    Point penetrationVector = intersection - _s1->m_center;
                    penetrationVector.normalize();
                    double penetrationDepth = Math::dist(intersection, _s1->m_points[i]);

                    return CollisionInfo_PCSCVX(true, intersection, penetrationVector, centerLine, collisionSurfaceNormal, penetrationDepth, _s1, _s2);
                }
            }
        }
    }
    return CollisionInfo_PCSCVX(false);
}

void Model::resolveCollisionPCSCVX(CollisionInfo_PCSCVX &collisionInfo)
{

    double ekTot = collisionInfo.s1->getE() + collisionInfo.s2->getE();

    double m_a = collisionInfo.s1->m_mass;
    double m_b = collisionInfo.s2->m_mass;

    double i_a = collisionInfo.s1->m_rotInert;
    double i_b = collisionInfo.s2->m_rotInert;

    double w_a1 = collisionInfo.s1->m_rot;
    double w_b1 = collisionInfo.s2->m_rot;

    Point n = collisionInfo.collisionSurfaceNormal;

    Point r_ap = collisionInfo.collisionPoint - collisionInfo.s1->m_center;
    Point r_bp = collisionInfo.collisionPoint - collisionInfo.s2->m_center;

    Point v_a1 = collisionInfo.s1->m_vel;
    Point v_b1 = collisionInfo.s2->m_vel;

    Point v_ap_rot = Math::instantVelRot2D(collisionInfo.collisionPoint, collisionInfo.s1->m_center, w_a1);
    Point v_ap1 = collisionInfo.s1->m_vel + v_ap_rot;

    Point v_bp_rot = Math::instantVelRot2D(collisionInfo.collisionPoint, collisionInfo.s2->m_center, w_b1);
    Point v_bp1 = collisionInfo.s2->m_vel + v_bp_rot;

    Point v_p1 = v_ap1 - v_bp1;

    double e = 1.0;
    double elasExpr = (1.0f + e) * -1;
    double numerator = Math::dotProd(v_p1 * elasExpr, n);
    double denominator = (1.0 / m_a) + (1.0 / m_b) + (Math::crossProdSquare(r_ap, n) / i_a) + (Math::crossProdSquare(r_bp, n) / i_b);
    double j = numerator / denominator;

    Point impulse = n * j;
    collisionInfo.s1->m_vel = v_a1 + (impulse / m_a);
    collisionInfo.s2->m_vel = v_b1 - (impulse / m_b);

    collisionInfo.s1->m_rot = w_a1 - (Math::crossProd3D(r_ap, impulse).z / i_a);
    collisionInfo.s2->m_rot = w_b1 + (Math::crossProd3D(r_bp, impulse).z / i_b);

    double ekTotF = collisionInfo.s1->getE() + collisionInfo.s2->getE();

    double diff = ekTotF - ekTot;

    if(fabs(diff) >  m_ENERGY_THRESHOLD)
    {
        std::cerr << "WARNING! ENERGY NOT CONSERVED AFTER SHAPE COLLISION! NET CHANGE (FINAL - INITIAL): " << diff << "\n";
    }

}

void Model::resolveCollisionOverlapPCSCVX(CollisionInfo_PCSCVX &collisionInfo)
{
    Point separation = (collisionInfo.penetrationVector * collisionInfo.penetrationDepth) * -1 * m_SEPARATION_SAFETY_FACTOR;
    collisionInfo.s1->moveShape(separation);
}

// Resolves initial collision by separating the object from the wall. Works by finding the distance between the point that collided
// into the wall and the wall itself along the objects velocity vector, and slides the shape back along this vector.
void Model::resolveOverlapCollisionPCSCVX_Wall_Linear(WallCollisionInfo_PCSCVX &wallCollisionInfo)
{
    Line slidingLine;

    // Gradient is the rise/run of the shapes velocity vector
    double gradient = wallCollisionInfo.shape->m_vel.y / wallCollisionInfo.shape->m_vel.x;
    // Collision point is just the point of the shape that came into contact with the wall
    Point collisionPoint = wallCollisionInfo.shape->m_points[wallCollisionInfo.pointIndex];
    // The equation of the line we want to slide the shape back along is the line with our calculated gradient, which passes through the collision point
    // If the velocity of the shape is only up or down the line is simply vertical
    if (wallCollisionInfo.shape->m_vel.x == 0 && wallCollisionInfo.shape->m_vel.y != 0)
    {
        slidingLine = Line(wallCollisionInfo.shape->m_points[wallCollisionInfo.pointIndex].x);
    }
    else
    {
        slidingLine = Line(gradient, collisionPoint);
    }
    // Obtain where this line intersects with the wall
    Point wallIntersection = Math::intersectionPt(slidingLine, Math::WALLS[wallCollisionInfo.wallSide]);
    // Change the position of the shape by the slideDelta
    Point slideDelta = (wallIntersection - collisionPoint) * m_SEPARATION_SAFETY_FACTOR;
    for (Point &p : wallCollisionInfo.shape->m_points)
    {
        p = p + slideDelta;
    }
    wallCollisionInfo.shape->m_center = wallCollisionInfo.shape->m_center + slideDelta;
}

void Model::resolveCollisionPCSCVX_Wall(WallCollisionInfo_PCSCVX &wci)
{

    // perfect elasticity
    double e = 1.0f;
    // X and Y components of the velocity of the collision point contributed to by rotation
    Point v_ap_rot = Math::instantVelRot2D(wci.shape->m_points[wci.pointIndex], wci.shape->m_center, wci.shape->m_rot);
    // Velocity of the collision point (shapes velocity + the points instantaneous rotational velocity)
    Point v_ap1 = wci.shape->m_vel + v_ap_rot;
    // Normal to the collision surface (collision surface is always the wall)
    Point n;
    switch (wci.wallSide)
    {
    case WALLSIDE::LEFT:
        n = {1.0f, 0.0f};
        break;
    case WALLSIDE::RIGHT:
        n = {-1.0f, 0.0f};
        break;
    case WALLSIDE::TOP:
        n = {0.0f, 1.0f};
        break;
    case WALLSIDE::BOTTOM:
        n = {0.0f, -1.0f};
        break;
    }

    // Vector pointing from the center of the shape to the collision point
    Point r_ap = wci.shape->m_points[wci.pointIndex] - wci.shape->m_center;
    // Mass, rotational inertia
    double m_a = wci.shape->m_mass;
    double i_a = wci.shape->m_rotInert;
    // Elasticity (e) part of the eq
    double elas = (1.0f + e) * -1;
    // -(1+e)*v_ap1 . n
    double numerator = Math::dotProd(v_ap1 * elas, n);
    double cp_2 = Math::crossProdSquare(r_ap, n);
    // (1/m_a) + (r_ap x n)^2 / I_a
    double denominator = (1.0f / m_a) + (cp_2 / i_a);

    // Impulse 'j' generated at the collision point
    double j = numerator / denominator;
    // Impulse acts in the opposite direction to the collision surface (wall)
    Point impulse = n * j;

    double initE = wci.shape->getE();
    wci.shape->m_vel = wci.shape->m_vel + (impulse / m_a);
    wci.shape->m_rot = wci.shape->m_rot - ((Math::crossProd3D(r_ap, impulse).z) / i_a);
    double endE = wci.shape->getE();

    if (fabs(endE - initE) > m_ENERGY_THRESHOLD)
    {
        std::cerr << "WARNING! ENERGY NOT CONSERVED AFTER WALL COLLISION! NET CHANGE (FINAL - INITIAL): " << (endE - initE) << "\n";
    }
}

// Returns shape ID's of potentially colliding shapes. Simple sweep and prune algorithm
std::vector<std::pair<PointCloudShape_Cvx &, PointCloudShape_Cvx &>> Model::isContactBroad()
{
    // Aspect ratios are universally in favor of a larger width. It is for this reason we will use the X-axis
    // to sweep as it provides a greater liklihood of eliminating shapes that won't collide.

    std::vector<std::pair<double, PointCloudShape_Cvx &>> intervals;

    return {};
}

WallCollisionInfo_PCSCVX Model::isContactWallLinear(PointCloudShape_Cvx &s1)
{
    // Check if any of the vertices go beyond the walls.
    bool leftCol = false, rightCol = false, bottomCol = false, topCol = false;
    for (int i = 0; i < s1.m_points.size(); i++)
    {
        leftCol = s1.m_points[i].x <= 0;
        rightCol = s1.m_points[i].x >= SCREEN_WIDTH;

        bottomCol = s1.m_points[i].y >= SCREEN_HEIGHT;
        topCol = s1.m_points[i].y <= 0;
        if (topCol || bottomCol || leftCol || rightCol)
            goto beginDot;
    }

    return WallCollisionInfo_PCSCVX(false);

beginDot:

    Point wallVec;
    int wallSide;
    if (topCol)
    {
        wallVec = Math::TOP_WALL_VEC;
        wallSide = WALLSIDE::TOP;
    }
    else if (bottomCol)
    {
        wallVec = Math::BOTTOM_WALL_VEC;
        wallSide = WALLSIDE::BOTTOM;
    }
    else if (leftCol)
    {
        wallVec = Math::LEFT_WALL_VEC;
        wallSide = WALLSIDE::LEFT;
    }
    else if (rightCol)
    {
        wallVec = Math::RIGHT_WALL_VEC;
        wallSide = WALLSIDE::RIGHT;
    }

    double closestPtDotProd = -std::numeric_limits<double>::max();
    double currentDotProd = 0;
    int closestPtIndx = 0;

    for (int i = 0; i < s1.m_points.size(); i++)
    {
        currentDotProd = Math::dotProd(wallVec, s1.m_points[i]);
        if (currentDotProd > closestPtDotProd)
        {
            closestPtIndx = i;
            closestPtDotProd = currentDotProd;
        }
    }

    return WallCollisionInfo_PCSCVX(true, wallSide, &s1, closestPtIndx, 0);
}
