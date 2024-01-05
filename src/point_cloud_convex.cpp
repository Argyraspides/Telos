#include "point_cloud_convex.h"
#include "shape_utils.h"
#include "engine_math.h"
#include "stdexcept"
#include <algorithm>

PointCloudShape_Cvx::PointCloudShape_Cvx() : Shape(SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE, BODY_TYPE_IDENTIFIERS::RIGID_BODY)
{
    m_center = Utils::getCentroid(m_points);
    m_rotInert = Utils::getRotInertia(m_points, m_mass);
}

PointCloudShape_Cvx::PointCloudShape_Cvx(const std::vector<Point> &points, Point vel, double rot, double mass, double timeSpawned) : Shape(SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE, BODY_TYPE_IDENTIFIERS::RIGID_BODY)
{
    m_points = points;
    m_vel = vel;
    m_rot = rot;
    m_mass = mass;
    m_initPoints = points;
    m_center = Utils::getCentroid(m_points);
    m_initPos = m_center;
    m_time = 0.0f;
    m_rotInert = Utils::getRotInertia(m_points, m_mass);
    m_timeSpawned = timeSpawned;

    if (m_points.size() > Utils::shapeNameCount)
        m_name = "Approximate Circle";
    else
        m_name = Utils::shapeNames[points.size()];

    for (int i = 0; i < points.size(); i++)
    {
        m_pointsRadial.push_back(Math::dist(m_points[i], m_center));
        m_Deltas.push_back(m_points[i] - m_center);
    }
}

std::vector<Point> PointCloudShape_Cvx::getPoints() const
{
    return m_points;
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

    static const double ENGINE_TIME_STEP = (1.0 / ENGINE_POLLING_RATE);

    double timeFrac = timeStep / ENGINE_TIME_STEP;

    double sinW = sin(timeDir * m_rot * timeFrac);
    double cosW = cos(timeDir * m_rot * timeFrac);

    double xDelta, yDelta;
    for (int i = 0; i < m_points.size(); i++)
    {
        // Motion of a point cloud polygon rotating around center (ox, oy). Each point (px, py), where the velocity is (vx, vy) equals:
        // px = (px - ox) • cos(wt) - (py - oy) • sin(wt) + ox + vx
        // py = (px - ox) • sin(wt) + (py - oy) • cos(wt) + oy + vy

        xDelta = m_points[i].x - m_center.x;
        yDelta = m_points[i].y - m_center.y;

        m_points[i].x = (xDelta * cosW - yDelta * sinW + m_center.x) + timeDir * m_vel.x * timeFrac;
        m_points[i].y = (xDelta * sinW + yDelta * cosW + m_center.y) + timeDir * m_vel.y * timeFrac;
    }
    m_center = m_center + m_vel * timeDir * timeFrac;
    m_time += timeStep * timeDir * timeFrac;
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

void PointCloudShape_Cvx::setShapeVel(const Point &p)
{
    m_vel = p;
}
