#include "shape.h"
#include <iostream>
#include <cmath>
#define MOVE_THRESH 1

Shape::Shape(int shapeTypeID)
{
    this->m_shapeTypeID = shapeTypeID;
    this->m_shapeID = (ID_CTR++);

    this->m_xVel = 0;
    this->m_yVel = 0;
    this->m_rot = 0;
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

// Checks if a point lies within a shape
bool ShapeUtils::isInside(Point p, std::shared_ptr<Shape> s)
{

    int shapeTypeID = s->getShapeTypeID();
    if (shapeTypeID == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {

        // Treating the point as the origin of a cartesian plane, if the polygon takes up all four quadrants, the point lies within the polygon
        // Remember that convention has it that the top left corner of the screen is the origin, and positive Y values are downwards from it (X values are still
        // positive going right and negative going left)

        std::shared_ptr<PointCloudShape_Cvx> pointCloudShape_Cvx = std::dynamic_pointer_cast<PointCloudShape_Cvx>(s);

        int quadrantCount[4] = {0, 0, 0, 0};

        std::vector<Point> shapePoints = pointCloudShape_Cvx->getPoints();

        for (int i = 0; i < shapePoints.size(); i++)
        {
            // Quadrants 1, 2, 3, and 4

            if (shapePoints[i].x < p.x && shapePoints[i].y < p.y)
                quadrantCount[0]++;

            else if (shapePoints[i].x > p.x && shapePoints[i].y < p.y)
                quadrantCount[1]++;

            else if (shapePoints[i].x < p.x && shapePoints[i].y > p.y)
                quadrantCount[2]++;

            else
                quadrantCount[3]++;
        }

        // If the points cover all four quadrants, the point is inside the polygon.
        bool isInside = quadrantCount[0] > 0 && quadrantCount[1] > 0 && quadrantCount[2] > 0 && quadrantCount[3] > 0;
        if (isInside)
        {
            std::cout << "INSIDE"
                      << "\n";
        }
        return isInside;
    }

    std::cerr << "SHAPE TYPE IS INVALID. ERROR IN FUNCTION: " << __func__ << " IN CLASS " << typeid(ShapeUtils).name() << std::endl;
    return false;
}

// Calculates the centroid of a polygon given its vertices (assumes that the polygon has an even mass distribution)
Point ShapeUtils::getCentroid(const std::vector<Point> &points)
{
    float sumX = 0.0, sumY = 0.0;
    for (const Point &p : points)
    {
        sumX += p.x;
        sumY += p.y;
    }
    return Point(sumX / (float)points.size(), sumY / (float)points.size());
}

// TODO: IMPLEMENT
bool ShapeUtils::checkConvex(const std::vector<Point> &points)
{
    for (int i = 0; i < points.size(); i++)
    {
    }

    return true;
}

void ShapeUtils::printInfo(PointCloudShape_Cvx s)
{
    std::cout << "SHAPE POINTS: ";
    for (Point &p : s.getPoints())
    {
        std::cout << "(" << p.x << "," << p.y << "),";
    }
    std::cout << "\n";
    std::cout << "CENTER: "
              << "(" << s.getCenter().x << "," << s.getCenter().y << ")\n";
}

// ***************************************************************************************************************************************************************
// CONSTRUCTORS (POINT CLOUD SHAPE)

PointCloudShape_Cvx::PointCloudShape_Cvx() : Shape(SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
{
    this->m_center = ShapeUtils::getCentroid(this->m_points);
    ShapeUtils::printInfo(*this);
}

PointCloudShape_Cvx::PointCloudShape_Cvx(const std::vector<Point> &points) : Shape(SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
{
    this->m_points = points;
    this->m_center = ShapeUtils::getCentroid(this->m_points);
    ShapeUtils::printInfo(*this);
}

// ***************************************************************************************************************************************************************
// GETTER FUNCTIONS

std::vector<Point> PointCloudShape_Cvx::getPoints() const
{
    return this->m_points;
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

// ***************************************************************************************************************************************************************
// SHAPE MODIFIERS

void PointCloudShape_Cvx::moveShape(const Point &p)
{
    for (Point &shapePts : this->m_points)
    {
        shapePts = shapePts + p;
    }
    this->m_center = this->m_center + p;
    ShapeUtils::printInfo(*this);
}

void PointCloudShape_Cvx::setShapePos(const Point &p)
{
    Point delta = this->m_center - p;
    delta = delta * -1;
    
    for (Point &shapePts : this->m_points)
    {
        shapePts = shapePts + delta;
    }

    this->m_center = this->m_center + delta;
}