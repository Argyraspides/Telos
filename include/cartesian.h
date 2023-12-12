#pragma once
// Defines basic data structures and math functions for anything related to a cartesian plane.

struct Point
{
    float x = 0, y = 0, z = 0;
    Point(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

    Point operator+(const Point &point) const
    {
        return {
            x + point.x,
            y + point.y,
            z + point.z};
    }

    Point operator-(const Point &point) const
    {
        return {
            x - point.x,
            y - point.y,
            z - point.z};
    }

    void operator=(const Point &point)
    {
        x = point.x;
        y = point.y;
        z = point.z;
    }

    Point
    operator/(const float &num) const
    {
        return {
            x / num,
            y / num,
            z / num};
    }

    Point operator*(const float &num) const
    {
        return {
            x * num,
            y * num,
            z * num};
    }
};

namespace Math
{
    static Point getNormal2D(Point p)
    {
        return {-p.y, p.x, p.z};
    }

    // TODO: IMPLEMENT
    static Point getNormal3D(Point p)
    {
        return {};
    }

    // TODO: IMPLEMENT
    static Point crossProd(Point p)
    {
        return {};
    }

    static float dotProd(const Point &p1, const Point &p2)
    {
        return (p1.x * p2.x) + (p1.y * p2.y) + (p1.z * p2.z);
    }
}
