#include "shape.h"
#include <iostream>
#include <cmath>
PointCloudShape_Cvx::PointCloudShape_Cvx() : Shape(SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
{
}

PointCloudShape_Cvx::PointCloudShape_Cvx(const std::vector<Point> &points) : Shape(SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
{
    this->m_points = points;
}

// ***************************************************************************************************************************************************************
// GETTER FUNCTIONS

Shape::Shape(int shapeTypeID)
{
    this->shapeTypeID = shapeTypeID;
    this->shapeID = (ID_CTR++);
}

std::vector<Point> PointCloudShape_Cvx::getPoints() const
{
    return this->m_points;
}

// ***************************************************************************************************************************************************************

// ***************************************************************************************************************************************************************
// CHECKER FUNCTIONS

bool PointCloudShape_Cvx::checkConvex(const std::vector<Point> &points)
{
    return true;
}

// ***************************************************************************************************************************************************************

std::vector<Point> PointCloudShape_Cvx::generateCircle(float radius)
{
    // x^2 + y^2 = r^2

    float increment = radius / (float) CIRCLE_POINT_COUNT;

    std::vector<Point> circle;
    circle.reserve(CIRCLE_POINT_COUNT * 2);

    Point pt;
    // y = +sqrt(r^2 - x^2)
    for (float x = -radius; x <= radius; x += increment)
    {
        pt.x = x;
        pt.y = sqrt(radius*radius - x*x);
        circle.push_back(pt);
    }

    // y = -sqrt(r^2 - x^2)
    for (float x = radius; x >= -radius; x -= increment)
    {
        pt.x = x;
        pt.y = -sqrt(radius*radius - x*x);
        circle.push_back(pt);
    }

    return circle;
}