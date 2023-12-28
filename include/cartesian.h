#pragma once
// Defines basic data structures and math functions for anything related to a cartesian plane.
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
        if (z > 0)
            z /= len;
    }

    double magnitude()
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

namespace Math
{
    // Origin of the 3D cartesian space (x, y, and z = 0)
    static Point origin = {0, 0, 0};

    // Position of the left, right, top and bottom walls, as well as their directional vectors
    static Line LEFT_WALL = {{0, SCREEN_HEIGHT, 0}, {0, 0, 0}};
    static Point LEFT_WALL_VEC = {-1, 0, 0};

    static Line RIGHT_WALL = {{SCREEN_WIDTH, SCREEN_HEIGHT, 0}, {SCREEN_WIDTH, 0, 0}};
    static Point RIGHT_WALL_VEC = {1, 0, 0};

    static Line TOP_WALL = {{0, 0, 0}, {SCREEN_WIDTH, 0, 0}};
    static Point TOP_WALL_VEC = {0, -1, 0};

    static Line BOTTOM_WALL = {{0, SCREEN_HEIGHT, 0}, {SCREEN_WIDTH, SCREEN_HEIGHT, 0}};
    static Point BOTTOM_WALL_VEC = {0, 1, 0};

    static Line WALLS[4] = {LEFT_WALL, TOP_WALL, RIGHT_WALL, BOTTOM_WALL};
    static Point WALL_VECS[4] = {LEFT_WALL_VEC, TOP_WALL_VEC, RIGHT_WALL_VEC, BOTTOM_WALL_VEC};

    // Calculates the normal of a vector in 2D space
    static Point getNormal2D(Point p)
    {
        return {-p.y, p.x, p.z};
    }

    // Calculates the normal of a 2D line
    static Line getNormal2D(Line l)
    {
        if (l.isVertical)
        {
            l.m = 0;
            l.x = 0;
            l.isVertical = false;
            return l;
        }

        l.m = -(1.0 / l.m);
        return l;
    }

    // TODO: IMPLEMENT
    static Point getNormal3D(Point p)
    {
        return {};
    }

    // Calcluates the cross product of two, 2D vectors is also known as a 'wedge product'
    static double crossProd2D(const Point &p1, const Point &p2)
    {
        return p1.x * p2.y - p1.y * p2.x;
    }

    // Calculates the cross product of two vectors (x1, y1, z1)
    // and (x2, y2, z2).
    static Point crossProd3D(const Point &p1, const Point &p2);
    static Point crossProd3D(const Point &p1, const Point &p2)
    {
        return {
            p2.y * p1.z - p2.z * p1.y,
            p2.z * p1.x - p2.x * p1.z,
            p2.x * p1.y - p2.y * p1.x};
    }

    // Calculates the dot product of two vectors
    static double dotProd(const Point &p1, const Point &p2);
    static double dotProd(const Point &p1, const Point &p2)
    {
        return (p1.x * p2.x) + (p1.y * p2.y) + (p1.z * p2.z);
    }

    // See https://proofwiki.org/wiki/Square_of_Vector_Cross_Product
    // Calculates the square of a cross product
    static double crossProdSquare(Point p1, Point p2)
    {
        // square of a vector cross product axb equals:
        // (a.a)(b.b) - (a.b)^2 where '.' is the dot product
        double ab = dotProd(p1, p2);
        return dotProd(p1, p1) * dotProd(p2, p2) - ab * ab;
    }

    // Calculates the distance between two points
    static double dist(const Point &p1, const Point &p2);
    static double dist(const Point &p1, const Point &p2)
    {
        return sqrt(
            pow((p1.x - p2.x), 2) +
            pow((p1.y - p2.y), 2) +
            pow((p1.z - p2.z), 2));
    }

    // Calculates the instantaneous velocity of a point "p" rotating about an axis "c"
    // Does not take into account any translational motion of the current point
    static Point instantVelRot2D(const Point &p, const Point &c, double rot)
    {
        Point r_cp = p - c;
        // The rotational axis in 2 dimensions is always perpendicular to the x-y plane
        return crossProd3D(r_cp, {0, 0, rot});
    }

    // Calculates the point of intersection of two lines (each y=mx + c, or x = c for a vertical line)
    static Point intersectionPt(const Line &l1, const Line &l2)
    {
        // y1 = m1*x + c1
        // y2 = m2*x + c2
        // m1*x + c1 = m2*x + c2
        // m1*x + c1 - m2*x - c2 = 0
        // x(m1 - m2) + c1 - c2 = 0

        // x_intersection = (c2 - c1) / (m1 - m2)
        // y_intersection = m1 * x_intersection + c1 or m2 * x_intersection + c2

        if (l1.isVertical)
        {
            // y2 = m2*x1 + c2
            return {l1.x, l2.m * l1.x + l2.c};
        }
        else if (l2.isVertical)
        {
            // y1 = m1*x2 + c1
            return {l2.x, l1.m * l2.x + l1.c};
        }

        double x_intersection = (l2.c - l1.c) / (l1.m - l2.m);
        return {x_intersection, l1.m * x_intersection + l1.c};
    }
}
