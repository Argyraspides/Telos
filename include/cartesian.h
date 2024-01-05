#pragma once
// Defines basic data structures for anything related to a cartesian plane, e.g. lines, points
#include <cmath>
#include "application_params.h"

struct Point
{
    double x = 0, y = 0, z = 0;
    Point(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}

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
    operator/(const double &num) const
    {
        return {
            x / num,
            y / num,
            z / num};
    }

    Point operator*(const double &num) const
    {
        return {
            x * num,
            y * num,
            z * num};
    }

    void normalize()
    {
        double len = sqrt(x * x + y * y + z * z);
        if (len == 0)
            return;

        x /= len;
        y /= len;
        z /= len;
    }

    double magnitude() const
    {
        return sqrt(x * x + y * y + z * z);
    }
};

struct Line
{
    bool isVertical = false;
    double m = 0, x = 0, c = 0;

    //  Constructs a line that passes through two points.
    Line(const Point &p1, const Point &p2)
    {
        // y = mx + c
        // m = (p1.y - p2.y) / (p1.x - p2.x)
        // p1.y = m*p1.x + c
        // p1.y - (m*p1.x) = c

        double rise = p1.y - p2.y;
        double run = p1.x - p2.x;
        double grad = rise / run;

        // Only have to check if run is too small to prevent dividing by zero errors (m = rise/run)
        if (std::isinf(grad))
        {
            x = p1.x;
            isVertical = true;
        }
        else
        {
            this->m = grad;
            if (rise == 0)
            {
                c = p1.y;
            }
            else
            {
                c = p1.y - (m * p1.x);
            }
        }
    }

    // Constructs a vertical line
    Line(double x)
    {
        this->x = x;
        isVertical = true;
    }

    // Constructs an ordinary line (non-vertical)
    Line(double m, double c)
    {
        this->m = m;
        this->c = c;
        isVertical = false;
    }

    // Constructs a line with gradient 'm', that intersects with a point 'p'
    Line(double m, Point p)
    {
        // y = mx + c
        // p.y = m*p.x + c
        // p.y - m*p.x = c
        // OR x = c, where c = p.x

        if (std::isinf(m))
        {
            this->isVertical = true;
            this->x = p.x;
        }
        else
        {
            this->m = m;
            this->c = p.y - m * p.x;
            this->isVertical = false;
        }
    }

    Line()
    {
    }

    void operator=(const Line &l)
    {
        m = l.m;
        x = l.x;
        c = l.c;
        isVertical = l.isVertical;
    }
};
