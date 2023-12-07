#include "shape.h"
#include <iostream>
#include <cmath>

Shape::Shape(int shapeTypeID)
{
    this->shapeTypeID = shapeTypeID;
    this->shapeID = (ID_CTR++);
}












// For any shape, resolves what kind of shape it is and then translates it into a point cloud for rendering
std::vector<Point> ShapeUtils::convertToPointCloud(std::shared_ptr<Shape> shape)
{
    int shapeTypeID = shape->getShapeTypeID();
    if (shapeTypeID == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        std::shared_ptr<PointCloudShape_Cvx> pointCloudShape_Cvx = std::dynamic_pointer_cast<PointCloudShape_Cvx>(shape);
        return pointCloudShape_Cvx->getPoints();
    }
    std::cerr << "SHAPE TYPE IS INVALID. ERROR IN FUNCTION: " << __func__ << " IN CLASS " << typeid(ShapeUtils).name() << std::endl;
    return {};
}

Point ShapeUtils::getCentroid(const std::vector<Point> &points)
{

    if (points.size() < 3)
        return {};

    int sectors = points.size() - 2;

    Point currentCentroidSum = {0, 0, 0};

    for (int i = 0; i < sectors; i++)
    {
        currentCentroidSum = currentCentroidSum + points[0] + points[i] + points[i + 1];
    }

    currentCentroidSum = currentCentroidSum / 3;
    return currentCentroidSum / (float)sectors;
}









// ***************************************************************************************************************************************************************
// CONSTRUCTORS

PointCloudShape_Cvx::PointCloudShape_Cvx() : Shape(SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
{
    this->center = ShapeUtils::getCentroid(this->m_points);
}

PointCloudShape_Cvx::PointCloudShape_Cvx(const std::vector<Point> &points) : Shape(SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
{
    this->center = ShapeUtils::getCentroid(this->m_points);
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
    float increment = 2 * radius / (float)CIRCLE_POINT_COUNT;

    std::vector<Point> circle;
    circle.reserve(CIRCLE_POINT_COUNT * 2);

    Point pt;
    // y = +sqrt(r^2 - x^2)
    for (float x = -radius; x <= radius; x += increment)
    {
        pt.x = x;
        pt.y = sqrt(radius * radius - x * x);
        circle.push_back(pt);
    }

    // y = -sqrt(r^2 - x^2)
    for (float x = radius; x >= -radius; x -= increment)
    {
        pt.x = x;
        pt.y = -sqrt(radius * radius - x * x);
        circle.push_back(pt);
    }

    return circle;
}