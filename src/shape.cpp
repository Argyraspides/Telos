#include "shape.h"
#include <iostream>
#include <cmath>
Shape::Shape(int shapeTypeID)
{
    this->shapeTypeID = shapeTypeID;
    this->shapeID = (ID_CTR++);
}


PointCloudShape_Cvx::PointCloudShape_Cvx() : Shape(SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
{
}

PointCloudShape_Cvx::PointCloudShape_Cvx(const std::vector<Point> &points) : Shape(SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
{
    this->m_points = points;
}

// ***************************************************************************************************************************************************************
// GETTER FUNCTIONS

std::vector<Point> PointCloudShape_Cvx::getPoints() const
{
    return this->m_points;
}

// ***************************************************************************************************************************************************************
// CHECKER FUNCTIONS

bool PointCloudShape_Cvx::checkConvex(const std::vector<Point> &points)
{
    return true;
}

// ***************************************************************************************************************************************************************
// SHAPE GENERATOR FUNCTIONS

std::vector<Point> PointCloudShape_Cvx::generateCircle(float radius)
{

    // Circle will be approximated with CIRCLE_POINT_COUNT number of points on each half
    float increment = 2*radius / (float) CIRCLE_POINT_COUNT;

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