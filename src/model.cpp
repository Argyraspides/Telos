#include "model.h"
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
        const std::chrono::milliseconds frameDuration((long)(timeStep * 1000));
        auto startTime = std::chrono::high_resolution_clock::now();

        while (m_isRunning)
        {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

            if (elapsed >= frameDuration && !m_isPaused)
            {
                // Update all shapes including their positions, resolve their collisions, etc
                updatePCSL();
                time += timeStep;
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
        this->m_PCSCVX_shapeList[i]->updateShape(timeStep);
    }

    for (int i = 0; i < size - 1; i++)
    {
        for (int j = i + 1; j < size; j++)
        {
            CollisionInfo_PCSCVX c = isContactPCSCVX_CL(*this->m_PCSCVX_shapeList[i], *this->m_PCSCVX_shapeList[j]);
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

// Uses the SAT (Separation Axis Theorem) to detect collision. Cannot determine collision point nor penetration depth.
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

            float min1 = std::numeric_limits<float>::infinity(),
                  max1 = -min1;

            // Obtain minimum and maximum points of projection for each shape

            for (int p = 0; p < _s1->m_points.size(); p++)
            {
                float dotProd = Math::dotProd(_s1->m_points[p], projectionAxis);
                min1 = std::min(min1, dotProd);
                max1 = std::max(max1, dotProd);
            }

            float min2 = std::numeric_limits<float>::infinity(),
                  max2 = -min2;

            for (int p = 0; p < _s2->m_points.size(); p++)
            {
                float dotProd = Math::dotProd(_s2->m_points[p], projectionAxis);
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

// Uses simple line intersections to detect collision, collision point, and penetration depth.
CollisionInfo_PCSCVX Model::isContactPCSCVX_CL(PointCloudShape_Cvx &s1, PointCloudShape_Cvx &s2)
{

    PointCloudShape_Cvx *_s1 = &s1;
    PointCloudShape_Cvx *_s2 = &s2;

    float edgeLineBoundsX[2] = {0, 0};
    float edgeLineBoundsY[2] = {0, 0};
    float centerLineBoundsX[2] = {0, 0};
    float centerLineBoundsY[2] = {0, 0};

    for (int s = 0; s < 2; s++)
    {
        if (s == 1)
        {
            _s1 = &s2;
            _s2 = &s1;
        }

        for (int i = 0; i < _s1->m_points.size(); i++)
        {

            Line centerLine(_s1->m_center, _s1->m_points[i]);

            centerLineBoundsY[0] = std::min(_s1->m_center.y, _s1->m_points[i].y);
            centerLineBoundsY[1] = std::max(_s1->m_center.y, _s1->m_points[i].y);

            centerLineBoundsX[0] = std::min(_s1->m_center.x, _s1->m_points[i].x);
            centerLineBoundsX[1] = std::max(_s1->m_center.x, _s1->m_points[i].x);

            for (int j = 0; j < _s2->m_points.size(); j++)
            {
                int wrap = (j + 1) % _s2->m_points.size();

                edgeLineBoundsY[0] = std::min(_s2->m_points[j].y, _s2->m_points[wrap].y);
                edgeLineBoundsY[1] = std::max(_s2->m_points[j].y, _s2->m_points[wrap].y);

                edgeLineBoundsX[0] = std::min(_s2->m_points[j].x, _s2->m_points[wrap].x);
                edgeLineBoundsX[1] = std::max(_s2->m_points[j].x, _s2->m_points[wrap].x);

                Line edgeLine(_s2->m_points[j], _s2->m_points[wrap]);
                Point intersection = Math::intersectionPt(centerLine, edgeLine);

                bool inCenterBoundsX = (intersection.x >= centerLineBoundsX[0] && intersection.x <= centerLineBoundsX[1]);
                bool inCenterBoundsY = (intersection.y >= centerLineBoundsY[0] && intersection.y <= centerLineBoundsY[1]);
                bool inEdgeBoundsX = (intersection.x >= edgeLineBoundsX[0] && intersection.x <= edgeLineBoundsX[1]);
                bool inEdgeBoundsY = (intersection.y >= edgeLineBoundsY[0] && intersection.y <= edgeLineBoundsY[1]);

                if (inCenterBoundsX && inCenterBoundsY && inEdgeBoundsX && inEdgeBoundsY)
                {
                    float penetrationDepth = Math::dist(intersection, _s1->m_points[i]);
                    std::shared_ptr<PointCloudShape_Cvx> ptr1 = std::make_shared<PointCloudShape_Cvx>(s1);
                    std::shared_ptr<PointCloudShape_Cvx> ptr2 = std::make_shared<PointCloudShape_Cvx>(s2);
                    return CollisionInfo_PCSCVX(true, intersection, {intersection - _s1->m_center}, centerLine, penetrationDepth, ptr1, ptr2);
                }
            }
        }
    }
    return CollisionInfo_PCSCVX(false);
}

void Model::resolveCollisionPCSCVX(CollisionInfo_PCSCVX collisionInfo)
{
}

// Resolves initial collision by separating the object from the wall. Works by finding the distance between the point that collided
// into the wall and the wall itself along the objects velocity vector, and slides the shape back along this vector.
void Model::resolveOverlapCollisionPCSCVX_Wall_Linear(WallCollisionInfo_PCSCVX wallCollisionInfo)
{
    Line slidingLine;

    // Gradient is the rise/run of the shapes velocity vector
    float gradient = wallCollisionInfo.shape->m_vel.y / wallCollisionInfo.shape->m_vel.x;
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
    Point slideDelta = wallIntersection - collisionPoint;
    for (Point &p : wallCollisionInfo.shape->m_points)
    {
        p = p + slideDelta;
    }
    wallCollisionInfo.shape->m_center = wallCollisionInfo.shape->m_center + slideDelta;
}

void Model::resolveCollisionPCSCVX_Wall(WallCollisionInfo_PCSCVX wci)
{

    // perfect elasticity
    float e = 1.0f;
    Point v_ap1 = wci.shape->m_vel;
    Point n;
    switch(wci.wallSide)
    {
        case WALLSIDE::LEFT:
            n = {1.0f,0.0f};
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
    Point r_ap = wci.shape->m_points[wci.pointIndex] - wci.shape->m_center;
    float m_a = wci.shape->m_mass;
    float i_a = wci.shape->m_rotInert;

    float elas = (1.0f + e) * -1;
    float numerator = Math::dotProd(v_ap1 * elas, n);
    float cp_2 = Math::crossProdSquare(r_ap, n);
    float denominator = (1.0f / m_a) + (cp_2 / i_a);

    float j = numerator / denominator;

    Point impulse = n * j;
    wci.shape->m_vel = wci.shape->m_vel + (impulse / m_a);
    wci.shape->m_rot = wci.shape->m_rot + ((-Math::crossProd3D(r_ap, impulse).z) / i_a);

    // translational kinetic energy (0.5mv^2)
    float ek = 0.5 * wci.shape->m_mass * wci.shape->m_vel.magnitude() * wci.shape->m_vel.magnitude(); 

    // rotational kinetic energy (0.5Iw^2)
    float ekRot = 0.5 * wci.shape->m_rotInert * pow(wci.shape->m_rot, 2);

    std::cout << ek + ekRot << "\n";
}

// Resolves initial collision by separating the object from the wall. Takes into account both linear translation as well as rotation of the object
void Model::resolveOverlapCollisionPCSCVX_Wall_LinearRot(WallCollisionInfo_PCSCVX wallCollisionInfo)
{

    wallCollisionInfo.shape->m_initPos = wallCollisionInfo.shape->m_center;
    ShapeUtils::printAllShapeInfo(*wallCollisionInfo.shape);

    // Motion of a point cloud polygon rotating around center (ox, oy). Each point (px, py), where the velocity is (vx, vy) equals:
    // px = (px - ox) • cos(w•t) - (py - oy) • sin(w•t) + ox + vx • t
    // py = (px - ox) • sin(w•t) + (py - oy) • cos(w•t) + oy + vy • t
    // Where the center is constantly moving (as the shape is constantly moving) as: o = i + v * t, where 'i' is the initial position of
    // the shape.
    // Therefore, t = (o - i)/v, or t = (ox - ix) / vx and t = (oy - iy) / vy
    // Substitute within cos(w•t) and sin(w•t) and we get the 'timeCollided' equations below, which is how long ago

    if (wallCollisionInfo.wallSide == WALLSIDE::TOP || wallCollisionInfo.wallSide == WALLSIDE::BOTTOM)
    {
        float yBoundary;
        if (wallCollisionInfo.wallSide == WALLSIDE::TOP)
            yBoundary = 0;
        else
            yBoundary = SCREEN_HEIGHT;

        float minTime = std::numeric_limits<float>::max();
        int collidedIndx = 0;

        // trig term = w • (oy - iy) / vy
        float trigTerm =
            wallCollisionInfo.shape->m_rot * ((wallCollisionInfo.shape->m_center.y - wallCollisionInfo.shape->m_center.y) / (wallCollisionInfo.shape->m_vel.y));
        // sin(w•t) and cos(w•t)
        float sintt = sin(trigTerm);
        float costt = cos(trigTerm);

        for (int i = 0; i < wallCollisionInfo.shape->m_points.size(); i++)
        {
            // timeCollided = (py - xd•sin(w•t) - yd•cos(w•t) - iy) / 2vy
            float xd = wallCollisionInfo.shape->m_points[i].x - wallCollisionInfo.shape->m_center.x;
            float xy = wallCollisionInfo.shape->m_points[i].y - wallCollisionInfo.shape->m_center.y;
            float numerator = (yBoundary - xd * sintt - xy * costt - wallCollisionInfo.shape->m_points[i].y);
            float denominator = wallCollisionInfo.shape->m_vel.y;
            float timeCollided = numerator / denominator;

            if (timeCollided < minTime)
            {
                minTime = timeCollided;
                collidedIndx = i;
            }
        }
        Point resolutionDist = wallCollisionInfo.shape->m_vel * minTime;
        float resolutionRot = wallCollisionInfo.shape->m_rot * minTime;

        wallCollisionInfo.shape->m_center = wallCollisionInfo.shape->m_center + resolutionDist;
        for (int i = 0; i < wallCollisionInfo.shape->m_points.size(); i++)
        {
            wallCollisionInfo.shape->m_points[i] = wallCollisionInfo.shape->m_points[i] + resolutionDist;
        }
        wallCollisionInfo.shape->rotShape(resolutionRot, wallCollisionInfo.shape->m_center);
    }
    else
    {
    }
    ShapeUtils::printAllShapeInfo(*wallCollisionInfo.shape);
    int x = 5;
}
// Returns shape ID's of potentially colliding shapes. Simple sweep and prune algorithm
std::vector<std::pair<PointCloudShape_Cvx &, PointCloudShape_Cvx &>> Model::isContactBroad()
{
    // Aspect ratios are universally in favor of a larger width. It is for this reason we will use the X-axis
    // to sweep as it provides a greater liklihood of eliminating shapes that won't collide.

    std::vector<std::pair<float, PointCloudShape_Cvx &>> intervals;

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

    float closestPtDotProd = -std::numeric_limits<float>::max();
    float currentDotProd = 0;
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

WallCollisionInfo_PCSCVX Model::isContactWallLinearRot(PointCloudShape_Cvx &s1)
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

    return WallCollisionInfo_PCSCVX(false);
}
