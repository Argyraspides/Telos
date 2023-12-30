#include "shape.h"
#include "engine_math.h"
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
    this->m_mass = 1.0f;
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

        // // Treating the point as the origin of a cartesian plane, if the polygon takes up all four quadrants, the point lies within the polygon

        // std::shared_ptr<PointCloudShape_Cvx> pointCloudShape_Cvx = std::dynamic_pointer_cast<PointCloudShape_Cvx>(s);

        // int quadrantCount[4] = {0, 0, 0, 0};

        // std::vector<Point> shapePoints = pointCloudShape_Cvx->getPoints();

        // for (int i = 0; i < shapePoints.size(); i++)
        // {
        //     // Quadrants 1, 2, 3, and 4 (not in order)

        //     double sx = shapePoints[i].x;
        //     double sy = shapePoints[i].y;

        //     if (sx < p.x && sy < p.y)
        //     {
        //         quadrantCount[0]++;
        //     }
        //     else if (sx < p.x && sy > p.y)
        //     {
        //         quadrantCount[1]++;
        //     }
        //     else if (sx > p.x && sy < p.y)
        //     {
        //         quadrantCount[2]++;
        //     }
        //     else if (sx > p.x && sy > p.y)
        //     {
        //         quadrantCount[3]++;
        //     }
        // }

        // // If the points cover all four quadrants, the point is inside the polygon.
        // bool isInside = quadrantCount[0] > 0 && quadrantCount[1] > 0 && quadrantCount[2] > 0 && quadrantCount[3] > 0;
        // std::cout << isInside << "\n";
        // return isInside ;

        std::shared_ptr<PointCloudShape_Cvx> pointCloudShape_Cvx = std::dynamic_pointer_cast<PointCloudShape_Cvx>(s);
        std::vector<Point> shapePoints = pointCloudShape_Cvx->getPoints();

        bool inside = false;
        for (int i = 0; i < shapePoints.size(); ++i)
        {
            const Point &v1 = shapePoints[i];
            const Point &v2 = shapePoints[(i + 1) % shapePoints.size()];

            if ((v1.y > p.y) != (v2.y > p.y) &&
                p.x < (v2.x - v1.x) * (p.y - v1.y) / (v2.y - v1.y) + v1.x)
            {
                inside = !inside;
            }
        }

        return inside;
    }

    std::cerr << "SHAPE TYPE IS INVALID. ERROR IN FUNCTION: " << __func__ << " IN CLASS " << typeid(ShapeUtils).name() << std::endl;
    return false;
}

// Calculates the centroid of a polygon given its vertices (assumes that the polygon has an even mass distribution)
Point ShapeUtils::getCentroid(const std::vector<Point> &points)
{
    // Assuming an even mass distribution the centroid is simply the average location of all the points
    double sumX = 0.0, sumY = 0.0;
    for (const Point &p : points)
    {
        sumX += p.x;
        sumY += p.y;
    }
    return Point(sumX / (double)points.size(), sumY / (double)points.size());
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
    for (int i = 0; i < s.m_points.size(); i++)
    {
        std::cout << "(" << s.m_points[i].x << "," << s.m_points[i].y << ")";
        if (i < s.m_points.size() - 1)
            std::cout << ",";
    }
    std::cout << ")\n";

    // std::cout //<< "CENTER: "
    //     << "(" << s.m_center.x << "," << s.m_center.y << ")\n";

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

// Obtain the rotational inertia, "I", of an arbitrary polygon
double ShapeUtils::getRotInertia(const std::vector<Point> &points)
{

    double j_x = 0, j_y = 0;

    for (int v = 0; v < points.size() - 1; v++)
    {
        int vpp = v + 1;
        // (x_i * y_i+1 - x_i+1 * y_i)
        double leftTerm =
            points[v].x * points[vpp].y - points[vpp].x * points[v].y;

        // (y_i^2 + y_i * y_i+1 + y_i+1^2)
        double rightTerm =
            pow(points[v].y, 2) + points[v].y * points[vpp].y + pow(points[vpp].y, 2);

        j_x += leftTerm + rightTerm;
        j_y += leftTerm;

        // (x_i^2 + x_i * x_i+1 + x_i+1^2)
        rightTerm =
            pow(points[v].x, 2) + points[v].x * points[vpp].x + pow(points[vpp].x, 2);

        j_y += rightTerm;
    }

    double oneTwelfth = 1.0f / 12.0f;
    j_x *= oneTwelfth;
    j_y *= oneTwelfth;

    return (j_x + j_y);

    // double jx = 0, jy = 0;

    // for (int i = 0; i < points.size(); i++)
    // {
    //     int wrap = (i + 1) % points.size();

    //     double prod1 = points[i].x * points[wrap].y - points[wrap].x * points[i].y;
    //     double prod2 = pow(points[i].y, 2) + points[i].y * points[wrap].y + pow(points[wrap].y, 2);

    //     jx += (prod1 * prod2);

    //     prod2 = pow(points[i].x, 2) + points[i].x * points[wrap].x + pow(points[wrap].x, 2);
    // }

    // double frac = 1.0f / 12.0f;
    // jx *= frac;
    // jy *= frac;

    // return (jx + jy);
}

// ***************************************************************************************************************************************************************
// CONSTRUCTORS (POINT CLOUD SHAPE)

PointCloudShape_Cvx::PointCloudShape_Cvx() : Shape(SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX, BODY_TYPE_IDENTIFIERS::RIGID_BODY)
{
    this->m_center = ShapeUtils::getCentroid(this->m_points);
    this->m_rotInert = ShapeUtils::getRotInertia(this->m_points);
}

PointCloudShape_Cvx::PointCloudShape_Cvx(const std::vector<Point> &points) : Shape(SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX, BODY_TYPE_IDENTIFIERS::RIGID_BODY)
{
    this->m_points = points;
    this->m_initPoints = points;
    this->m_center = ShapeUtils::getCentroid(this->m_points);
    this->m_initPos = m_center;
    this->m_time = 0.0f;
    this->m_rotInert = ShapeUtils::getRotInertia(this->m_points);

    for (int i = 0; i < points.size(); i++)
    {
        this->m_pointsRadial.push_back(Math::dist(m_points[i], this->m_center));
        this->m_Deltas.push_back(this->m_points[i] - this->m_center);
    }
}

// ***************************************************************************************************************************************************************
// GETTER FUNCTIONS

std::vector<Point> PointCloudShape_Cvx::getPoints() const
{
    return this->m_points;
}

double PointCloudShape_Cvx::getEkrot()
{
    // 0.5 * I * w^2
    return 0.5 * m_rotInert * m_rot * m_rot;
}

double PointCloudShape_Cvx::getEk()
{
    // 0.5 * m * v^2
    double v = m_vel.magnitude();
    return 0.5 * m_mass * v * v;
}

double PointCloudShape_Cvx::getE()
{
    return getEk() + getEkrot();
}

// ***************************************************************************************************************************************************************
// SHAPE GENERATOR FUNCTIONS

std::vector<Point> PointCloudShape_Cvx::generateRegularPolygon(double radius, int sides)
{
    double increment = 2.0 * M_PI / (double)sides;
    std::vector<Point> regularPolygon;
    regularPolygon.reserve(sides);

    double start = -M_PI/2.0;

    for(int i = 0; i < sides; i++)
    {
        double ang = start + (i * increment);
        regularPolygon.push_back({radius*cos(ang), radius*sin(ang), 0});
    }
    return regularPolygon;   
}

std::vector<Point> PointCloudShape_Cvx::generateRectangle(double w, double h)
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
    for (int i = 0; i < m_points.size(); i++)
    {
        m_points[i] = m_points[i] + p;
    }
    m_center = m_center + p;
    m_initPos = m_center;
    m_initPoints = m_points;
}

void PointCloudShape_Cvx::setShapePos(const Point &p)
{
    Point delta = m_center - p;
    delta = delta * -1;

    for (Point &shapePts : m_points)
    {
        shapePts = shapePts + delta;
    }

    m_center = m_center + delta;
}

void PointCloudShape_Cvx::updateShape(const double &timeStep, const int &timeDir)
{
    double sinW = sin(timeDir * m_rot);
    double cosW = cos(timeDir * m_rot);

    double xDelta, yDelta;
    for (int i = 0; i < m_points.size(); i++)
    {
        // Motion of a point cloud polygon rotating around center (ox, oy). Each point (px, py), where the velocity is (vx, vy) equals:
        // px = (px - ox) • cos(wt) - (py - oy) • sin(wt) + ox + vx
        // py = (px - ox) • sin(wt) + (py - oy) • cos(wt) + oy + vy

        xDelta = m_points[i].x - m_center.x;
        yDelta = m_points[i].y - m_center.y;

        m_points[i].x = (xDelta * cosW - yDelta * sinW + m_center.x) + timeDir * m_vel.x;
        m_points[i].y = (xDelta * sinW + yDelta * cosW + m_center.y) + timeDir * m_vel.y;
    }
    m_center = m_center + m_vel * timeDir;
    m_time += timeStep * timeDir;
}

void PointCloudShape_Cvx::rotShape(const double &rad, const Point &pivot)
{
    for (Point &v : m_points)
    {
        v = v - m_center;

        double sRot = sin(m_rot);
        double cRot = cos(m_rot);

        Point shift =
            {
                v.x * cRot - v.y * sRot,
                v.x * sRot + v.y * cRot};

        v = shift + m_center;
    }
}