#include "model.h"
#include "shape_utils.h"
#include "application_params.h"
#include <chrono>
#include <iostream>
#include <algorithm>

Model::Model()
{
    m_shapeType = SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE;
    m_bodyType = BODY_TYPE_IDENTIFIERS::RIGID_BODY;

    pthread_mutex_init(&shapeListMutex, nullptr);
    pthread_mutex_init(&PCSCVXShapeListMutex, nullptr);
}

Model::~Model()
{
    pthread_mutex_destroy(&shapeListMutex);
    pthread_mutex_destroy(&PCSCVXShapeListMutex);
}

// The engine can only handle shapes of one particular data structure at a time. Here we resolve what
// kind of shapes we are dealing with and then begin the respective loop that handles those particular
// types of shapes. By default the shape type is of type point cloud.
void Model::run()
{
    m_isRunning = true;
    m_isPaused = false;
    if (m_shapeType == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE && m_bodyType == BODY_TYPE_IDENTIFIERS::RIGID_BODY)
    {
        // Control the loop to execute at the desired frequency
        m_currentUpdateFunc = &Model::updatePCSCVXList;
        (this->*m_currentUpdateFunc)(TIME_DIRECTION::FORWARD);
        const std::chrono::milliseconds frameDuration((long)(m_timeStep * 1000));
        auto startTime = std::chrono::high_resolution_clock::now();

        while (m_isRunning)
        {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

            if (elapsed >= frameDuration && !m_isPaused)
            {
                // Update all shapes including their positions, resolve their collisions, etc
                updatePCSCVXList(TIME_DIRECTION::FORWARD);
                startTime = std::chrono::high_resolution_clock::now();
            }
        }
    }
    else if (m_shapeType == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE && m_bodyType == BODY_TYPE_IDENTIFIERS::PARTICLE)
    {
        while (m_isRunning)
        {
        }
    }
}
// Update Point Cloud Shape List
void Model::updatePCSCVXList(int timeDirection)
{
    m_time += (m_timeStep * timeDirection);

    // Calling "getPCSCVXShapeList()" locks access to the shape list.
    int size = getPCSCVXShapeList().size();

    for (int i = 0; i < m_PCSCVX_shapeList.size(); i++)
    {
        m_PCSCVX_shapeList[i]->updateShape(m_timeStep, timeDirection);
    }

    auto collidingPairs = isContactBroadPCSVX();
    for (std::pair<PointCloudShape_Cvx &, PointCloudShape_Cvx &> pair : collidingPairs)
    {
        CollisionInfo_PCSCVX c = isContactPCSCVX_CL(pair.first, pair.second);
        if (c.hasCollided)
        {
            resolveCollisionOverlapPCSCVX(c);
            resolveCollisionPCSCVX(c);
        }
    }

    for (int i = 0; i < size; i++)
    {
        bool col = QuickIsContactWall(*m_PCSCVX_shapeList[i]);
        if (col)
        {
            // Resolve the collision overlap taking into account both rotation and translation
            resolveCollisionOverlapPCSCVX_Wall_Rot(*m_PCSCVX_shapeList[i]);
            // Find the collision point
            WallCollisionInfo_PCSCVX wci = isContactWall(*m_PCSCVX_shapeList[i]);
            // Pull away from the wall to avoid the shape getting stuck
            resolveCollisionOverlapPCSCVX_Wall(wci);
            // Resolve the wall collision 
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
    return m_shapeList;
}
// MAKE SURE TO CALL pthread_mutex_unlock() ONCE YOU ARE DONE ACCESSING THE LIST
std::vector<std::shared_ptr<PointCloudShape_Cvx>> &Model::getPCSCVXShapeList()
{
    pthread_mutex_lock(&PCSCVXShapeListMutex);
    return m_PCSCVX_shapeList;
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

    double elasExpr = (1.0f + m_collisionElasticity) * -1;
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

    if (fabs(diff) > m_ENERGY_THRESHOLD && m_collisionElasticity == 1.0)
    {
        std::cout << "WARNING! ENERGY NOT CONSERVED AFTER SHAPE COLLISION! NET CHANGE (FINAL - INITIAL): " << diff << "\n";
    }
}

void Model::resolveCollisionOverlapPCSCVX(CollisionInfo_PCSCVX &collisionInfo)
{
    Point separation = (collisionInfo.penetrationVector * collisionInfo.penetrationDepth); //* m_SEPARATION_SAFETY_FACTOR;

    // Prefer to move the shape with lower mass
    if (collisionInfo.s1->m_mass > collisionInfo.s2->m_mass)
    {
        collisionInfo.s2->moveShape(separation);
    }
    else
    {
        collisionInfo.s1->moveShape(separation * -1);
    }
}

void Model::resolveCollisionOverlapPCSCVX_Rot(CollisionInfo_PCSCVX &collisionInfo)
{
}

// Resolves initial collision by separating the object from the wall. Works by finding the distance between the point that collided
// into the wall and the wall itself along the objects velocity vector, and slides the shape back along this vector. Multi wall collisions,
// I.e. a shape that collides on two walls at once (e.g. a slanted rectangle touching the top and left hand side) are handled as well.
// Shapes that collide with opposite walls are not handled and hence the response won't be accurate.
void Model::resolveCollisionOverlapPCSCVX_Wall(WallCollisionInfo_PCSCVX &wci)
{
    Point collisionPoint[2];
    collisionPoint[0] = wci.m_collisionPoint;
    if (wci.m_multiWall)
    {
        Point slideVec = wci.m_collisionNormal;

        Line l1((slideVec.y / slideVec.x), wci.m_collisionPoint);
        Line l2((slideVec.y / slideVec.x), wci.m_secondCollisionPoint);

        Point instc1 = Math::intersectionPt(l1, Math::WALLS[wci.m_wallSide]);
        Point instc2 = Math::intersectionPt(l2, Math::WALLS[wci.m_secondWallSide]);

        double dist1 = Math::dist(wci.m_collisionPoint, instc1);
        double dist2 = Math::dist(wci.m_secondCollisionPoint, instc2);

        double slideDist = std::max(dist1, dist2);

        slideVec = slideVec * slideDist;
        wci.m_shape->moveShape(slideVec);
    }
    else
    {
        Point slideVec = wci.m_shape->m_center - wci.m_collisionPoint;
        slideVec.normalize();
        Line slidingLine(wci.m_shape->m_center, wci.m_collisionPoint);
        Point wallIntsct = Math::intersectionPt(slidingLine, Math::WALLS[wci.m_wallSide]);
        double slideDist = Math::dist(wallIntsct, wci.m_collisionPoint);
        wci.m_shape->moveShape(slideVec * slideDist);
    }
}


// Performs a binary search to converge towards when exactly the collision occured, and moves 
// the shape back in time by the calculated amount. The shape is then pulled toward the wall so that there is a 
// tiny bit of overlap such that a collision point can be found
void Model::resolveCollisionOverlapPCSCVX_Wall_Rot(PointCloudShape_Cvx &s1)
{
    double timeStep = m_timeStep / 2.0;
    int stepDirection = TIME_DIRECTION::BACKWARD;

    for (int i = 0; i < m_wallOverlapResolution; i++)
    {
        s1.updateShape(timeStep, stepDirection);

        if (QuickIsContactWall(s1))
        {
            stepDirection = TIME_DIRECTION::BACKWARD;
        }
        else
        {
            stepDirection = TIME_DIRECTION::FORWARD;
        }
        timeStep /= 2.0;
    }

    if (!QuickIsContactWall(s1))
    {
        s1.updateShape(timeStep * 2.0, TIME_DIRECTION::FORWARD);
    }
}

void Model::resolveCollisionPCSCVX_Wall(WallCollisionInfo_PCSCVX &wci)
{

    // X and Y components of the velocity of the collision point contributed to by rotation
    Point v_ap_rot = Math::instantVelRot2D(wci.m_collisionPoint, wci.m_shape->m_center, wci.m_shape->m_rot);
    // Velocity of the collision point (shapes velocity + the points instantaneous rotational velocity)
    Point v_ap1 = wci.m_shape->m_vel + v_ap_rot;
    // Normal to the collision surface (collision surface is always the wall)
    Point n = wci.m_collisionNormal;

    // Vector pointing from the center of the shape to the collision point
    Point r_ap = wci.m_collisionPoint - wci.m_shape->m_center;
    // Mass, rotational inertia
    double m_a = wci.m_shape->m_mass;
    double i_a = wci.m_shape->m_rotInert;
    // Elasticity (e) part of the eq
    double elasExpr = (1.0f + m_wallCollisionElasticity) * -1;
    // -(1+e)*v_ap1 . n
    double numerator = Math::dotProd(v_ap1 * elasExpr, n);
    double cp_2 = Math::crossProdSquare(r_ap, n);
    // (1/m_a) + (r_ap x n)^2 / I_a
    double denominator = (1.0f / m_a) + (cp_2 / i_a);

    // Impulse 'j' generated at the collision point
    double j = numerator / denominator;
    // Impulse acts in the opposite direction to the collision surface (wall)
    Point impulse = n * j;

    double initE = wci.m_shape->getE();
    wci.m_shape->m_vel = wci.m_shape->m_vel + (impulse / m_a);
    wci.m_shape->m_rot = wci.m_shape->m_rot - ((Math::crossProd3D(r_ap, impulse).z) / i_a);
    double endE = wci.m_shape->getE();

    if (fabs(endE - initE) > m_ENERGY_THRESHOLD && m_wallCollisionElasticity == 1.0)
    {
        std::cout << "WARNING! ENERGY NOT CONSERVED AFTER WALL COLLISION! NET CHANGE (FINAL - INITIAL): " << (endE - initE) << "\n";
    }
}

// Returns pairs of potentially colliding shapes. Simple sweep and prune algorithm
std::vector<std::pair<PointCloudShape_Cvx &, PointCloudShape_Cvx &>> Model::isContactBroadPCSVX()
{
    // Aspect ratios are universally in favor of a larger width. It is for this reason we will use the X-axis
    // to sweep as it provides a greater liklihood of eliminating shapes that won't collide.

    std::vector<std::pair<PointCloudShape_Cvx &, PointCloudShape_Cvx &>> collidingPairs;
    collidingPairs.reserve(m_PCSCVX_shapeList.size() * 2);

    static auto lambda = [](const Point &a, const Point &b)
    { return a.x < b.x; };

    static auto pairLambda = [](const std::pair<double, int> &p1, const std::pair<double, int> &p2)
    { return p1.first < p2.first; };

    std::vector<std::pair<double, int>> intervals;
    intervals.reserve(m_PCSCVX_shapeList.size() * 2);

    for (int i = 0; i < m_PCSCVX_shapeList.size(); i++)
    {
        auto min = std::min_element(m_PCSCVX_shapeList[i]->m_points.begin(), m_PCSCVX_shapeList[i]->m_points.end(), lambda);
        auto max = std::max_element(m_PCSCVX_shapeList[i]->m_points.begin(), m_PCSCVX_shapeList[i]->m_points.end(), lambda);
        intervals.push_back({min->x, i});
        intervals.push_back({max->x, i});
    }

    std::sort(intervals.begin(), intervals.end(), pairLambda);

    for (int i = 0; i < intervals.size(); i++)
    {
        for (int j = i + 1; j < intervals.size(); j++)
        {
            if (intervals[j].second == intervals[i].second)
            {
                intervals.erase(intervals.begin() + j);
                break;
            }
            else
            {
                collidingPairs.push_back({*m_PCSCVX_shapeList[intervals[i].second], *m_PCSCVX_shapeList[intervals[j].second]});
            }
        }
    }

    return collidingPairs;
}

WallCollisionInfo_PCSCVX Model::isContactWall(PointCloudShape_Cvx &s1)
{
    // Check if any of the vertices go beyond the walls.
    bool leftCol = false, rightCol = false, bottomCol = false, topCol = false;

    std::vector<std::pair<Point, int>> colPoints;
    colPoints.reserve(s1.m_points.size());

    for (int i = 0; i < s1.m_points.size(); i++)
    {
        leftCol = s1.m_points[i].x <= 0;
        rightCol = s1.m_points[i].x >= SCREEN_WIDTH;

        bottomCol = s1.m_points[i].y >= SCREEN_HEIGHT;
        topCol = s1.m_points[i].y <= 0;

        if (topCol)
        {
            colPoints.push_back({s1.m_points[i], WALL_SIDE::TOP});
        }
        else if (bottomCol)
        {
            colPoints.push_back({s1.m_points[i], WALL_SIDE::BOTTOM});
        }
        else if (leftCol)
        {
            colPoints.push_back({s1.m_points[i], WALL_SIDE::LEFT});
        }
        else if (rightCol)
        {
            colPoints.push_back({s1.m_points[i], WALL_SIDE::RIGHT});
        }
    }

    // No collision
    if (colPoints.size() == 0)
    {
        return WallCollisionInfo_PCSCVX(false);
    }
    // Collision of only a single point
    else if (colPoints.size() == 1)
    {
        return WallCollisionInfo_PCSCVX(true, false, colPoints[0].second, &s1, colPoints[0].first);
    }
    // Flat collision, we take the average of the two points
    else if (colPoints.size() == 2)
    {
        if (colPoints[0].second != colPoints[1].second)
        {
            // if the collision points are on different walls, this must be handled in a special manner. Return both points.
            return WallCollisionInfo_PCSCVX(true, true, colPoints[0].second, &s1, colPoints[0].first, colPoints[1].first, colPoints[1].second);
        }
        Point avg = (colPoints[0].first + colPoints[1].first) / 2.0;
        return WallCollisionInfo_PCSCVX(true, false, colPoints[0].second, &s1, avg);
    }
    // GGWP. In this scenario we will just take the point that is furthest away from the wall.
    double closestPtDotProd = -std::numeric_limits<double>::max();
    double currentDotProd = 0;
    Point closestPoint = 0;

    for (int i = 0; i < s1.m_points.size(); i++)
    {
        // We will take the side of the wall as whatever the first collison point was
        currentDotProd = Math::dotProd(Math::WALL_VECS[colPoints[0].second], s1.m_points[i]);
        if (currentDotProd > closestPtDotProd)
        {
            closestPoint = s1.m_points[i];
            closestPtDotProd = currentDotProd;
        }
    }
    return WallCollisionInfo_PCSCVX(true, false, colPoints[0].second, &s1, closestPoint);
}

// Tells if a shape has collided with the wall without computing any collision information.
bool Model::QuickIsContactWall(PointCloudShape_Cvx &s1)
{
    bool leftCol, rightCol, bottomCol, topCol;
    for (int i = 0; i < s1.m_points.size(); i++)
    {
        leftCol = s1.m_points[i].x <= 0;
        rightCol = s1.m_points[i].x >= SCREEN_WIDTH;

        bottomCol = s1.m_points[i].y >= SCREEN_HEIGHT;
        topCol = s1.m_points[i].y <= 0;

        if (topCol || bottomCol || rightCol || leftCol)
            return true;
    }
    return false;
}