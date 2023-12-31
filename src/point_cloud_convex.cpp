#include "point_cloud_convex.h"
#include "shape_utils.h"
#include "engine_math.h"
#include <algorithm>

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

    double start = -M_PI / 2.0;

    for (int i = 0; i < sides; i++)
    {
        double ang = start + (i * increment);
        regularPolygon.push_back({radius * cos(ang), radius * sin(ang), 0});
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