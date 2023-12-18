#include "model.h"
#include "application_params.h"
#include <chrono>
#include <iostream>

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
        const std::chrono::milliseconds frameDuration(1000 / ENGINE_POLLING_RATE);
        auto startTime = std::chrono::high_resolution_clock::now();

        while (m_isRunning)
        {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

            if (elapsed >= frameDuration && !m_isPaused)
            {
                // Update all shapes including their positions, resolve their collisions, etc
                updatePCSL();
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
        this->m_PCSCVX_shapeList[i]->moveShape(m_PCSCVX_shapeList[i]->m_vel);
    }

    for (int i = 0; i < size; i++)
    {
        this->m_PCSCVX_shapeList[i]->rotShape(m_PCSCVX_shapeList[i]->m_rot, m_PCSCVX_shapeList[i]->m_center);
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
        int icw = isContactWall(*this->m_PCSCVX_shapeList[i]);
        if (icw == WALLSIDE::TOP_BOTTOM)
        {
            this->m_PCSCVX_shapeList[i]->m_vel.y *= -1;
        }
        else if (icw == WALLSIDE::LEFT_RIGHT)
        {
            this->m_PCSCVX_shapeList[i]->m_vel.x *= -1;
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

void Model::resolveCollisionPCSCVX_Wall(PointCloudShape_Cvx &s, int wallSide)
{

}

// Returns shape ID's of potentially colliding shapes. Simple sweep and prune algorithm
std::vector<std::pair<PointCloudShape_Cvx &, PointCloudShape_Cvx &>> Model::isContactBroad()
{
    // Aspect ratios are universally in favor of a larger width. It is for this reason we will use the X-axis
    // to sweep as it provides a greater liklihood of eliminating shapes that won't collide.

    std::vector<std::pair<float, PointCloudShape_Cvx &>> intervals;

    return {};
}

// Returns if shape has contacted wall. 0 is no contact, 1 is top/bottom wall, 2 is left/right wall
int Model::isContactWall(const PointCloudShape_Cvx &s1)
{
    // Check if any of the vertices go beyond the walls

    for (int i = 0; i < s1.m_points.size(); i++)
    {
        bool leftCol = s1.m_points[i].x <= 0;
        bool rightCol = s1.m_points[i].x >= SCREEN_WIDTH;

        bool bottomCol = s1.m_points[i].y >= SCREEN_HEIGHT;
        bool topCol = s1.m_points[i].y <= 0;

        if(bottomCol || topCol)
        {
            return WALLSIDE::TOP_BOTTOM;
        }
        if (leftCol || rightCol)
        {
            return WALLSIDE::LEFT_RIGHT;
        }
    }

    return WALLSIDE::NONE;
}
