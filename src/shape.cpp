#include "shape.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#define MOVE_THRESH 1
long long Shape::ID_CTR = 0;

Shape::Shape(int shapeTypeID, int bodyTypeID)
{
    this->m_shapeTypeID = shapeTypeID;
    this->m_bodyTypeID = bodyTypeID;
    this->m_shapeID = (ID_CTR++);
    this->m_vel = {0, 0, 0};
    this->m_rot = 0;
}

// For any shape, resolves what kind of shape it is and then translates it into a point cloud for rendering
std::vector<Point> ShapeUtils::convertToPointCloud(const std::shared_ptr<Shape> &shape)
{
    int shapeTypeID = shape->getShapeTypeID();
    if (shapeTypeID == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        // Point cloud shapes are already of type point cloud. Just return points.
        std::shared_ptr<PointCloudShape_Cvx> pointCloudShape_Cvx = std::dynamic_pointer_cast<PointCloudShape_Cvx>(shape);
        return pointCloudShape_Cvx->getPoints();
    }
    std::cerr << "SHAPE TYPE IS INVALID. ERROR IN FUNCTION: " << __func__ << " IN CLASS " << typeid(ShapeUtils).name() << std::endl;
    return {};
}

// Checks if a point lies within a shape
bool ShapeUtils::isInside(Point p, const std::shared_ptr<Shape> &s)
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

        for (size_t i = 0; i < shapePoints.size(); i++)
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
        return quadrantCount[0] > 0 && quadrantCount[1] > 0 && quadrantCount[2] > 0 && quadrantCount[3] > 0;
    }

    std::cerr << "SHAPE TYPE IS INVALID. ERROR IN FUNCTION: " << __func__ << " IN CLASS " << typeid(ShapeUtils).name() << std::endl;
    return false;
}

// Calculates the centroid of a polygon given its vertices (assumes that the polygon has an even mass distribution)
Point ShapeUtils::getCentroid(const std::vector<Point> &points)
{
    // Assuming an even mass distribution the centroid is simply the average location of all the points
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
    for (size_t i = 0; i < points.size(); i++)
    {
    }

    return true;
}

void ShapeUtils::printAllShapeInfo(PointCloudShape_Cvx s)
{
    std::cout << "polygon(";
    for(int i = 0; i < s.m_points.size(); i++)
    {
        std::cout << "(" << s.m_points[i].x << "," << s.m_points[i].y << ")";
        if(i < s.m_points.size() - 1) std::cout << ",";
    }
    std::cout << ")\n";

    // std::cout << "CENTER: "
    //           << "(" << s.m_center.x << "," << s.m_center.y << ")\n";

    // std::cout << "VELOCITY: (" << s.m_vel.x << "," << s.m_vel.y << ")\n";

    // std::cout << "\n";
}

void ShapeUtils::printLineInfo(Line l)
{
    if (!l.isVertical)
        std::cout << "y = " << l.m << "x + " << l.c << "\n";
    else
        std::cout << "x = " << l.x << "\n";
}
void ShapeUtils::printPointInfo(Point p)
{
    std::cout << "(" << p.x << "," << p.y << ")\n";
}

// ***************************************************************************************************************************************************************
// CONSTRUCTORS (POINT CLOUD SHAPE)

PointCloudShape_Cvx::PointCloudShape_Cvx() : Shape(SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX, BODY_TYPE_IDENTIFIERS::RIGID_BODY)
{
    this->m_center = ShapeUtils::getCentroid(this->m_points);
}

PointCloudShape_Cvx::PointCloudShape_Cvx(const std::vector<Point> &points) : Shape(SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX, BODY_TYPE_IDENTIFIERS::RIGID_BODY)
{
    this->m_points = points;
    this->m_center = ShapeUtils::getCentroid(this->m_points);
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

std::vector<Point> PointCloudShape_Cvx::generateRectangle(float w, float h)
{
    return {
        {0, 0},
        {w, 0},
        {w, h},
        {0, h}};
}

std::vector<Point> PointCloudShape_Cvx::generateTriangle(Point p1, Point p2, Point p3)
{
    std::vector<Point> pts = {p1, p2, p3};
    std::sort(pts.begin(), pts.end(), [](const Point &a, const Point &b)
              { return a.x < b.x; });
    return pts;
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

void PointCloudShape_Cvx::rotShape(const float &rad, const Point &pivot)
{
}