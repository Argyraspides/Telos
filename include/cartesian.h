#pragma once
// Defines basic data structures and math functions for anything related to a cartesian plane.
#include <cmath>
#include "application_params.h"
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

struct Line
{
    // x exists as a means to tell if the line is completely vertical or not
    bool isVertical = false;
    float m = 0, x = 0, c = 0;
    const float VERTICAL_THRESHOLD = 0.00001;

    //  Constructs a line that passes through two points.
    Line(const Point &p1, const Point &p2)
    {
        // y = mx + c
        // m = (p1.y - p2.y) / (p1.x - p2.x)
        // p1.y = m*p1.x + c
        // p1.y - (m*p1.x) = c

        float rise = p1.y - p2.y;
        float run = p1.x - p2.x;

        // Only have to check if run is too small to prevent dividing by zero errors (m = rise/run)
        if (abs(run - 0) < VERTICAL_THRESHOLD)
        {
            x = p1.x;
            isVertical = true;
        }
        else
        {
            this->m = rise / run;
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
    Line(float x)
    {
        this->x = x;
        isVertical = true;
    }

    // Constructs an ordinary line (non-vertical)
    Line(float m, float c)
    {
        this->m = m;
        this->c = c;
        isVertical = false;
    }

    // Constructs a line with gradient 'm', that intersects with a point 'p'
    Line(float m, Point p)
    {
        // y = mx + c
        // p.y = m*p.x + c
        // p.y - m*p.x = c
        this->m = m;
        this->c = p.y - m * p.x;
        this->isVertical = false;
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
    static Point origin = {0, 0, 0};

    static Line LEFT_WALL = {{0, SCREEN_HEIGHT, 0}, {0, 0, 0}};
    static Point LEFT_WALL_VEC = {-1,0,0};

    static Line RIGHT_WALL = {{SCREEN_WIDTH, SCREEN_HEIGHT, 0}, {SCREEN_WIDTH, 0, 0}};
    static Point RIGHT_WALL_VEC = {1,0,0};

    static Line TOP_WALL = {{0, 0, 0}, {SCREEN_WIDTH, 0, 0}};
    static Point TOP_WALL_VEC = {0,-1,0};

    static Line BOTTOM_WALL = {{0, SCREEN_HEIGHT, 0}, {SCREEN_WIDTH, SCREEN_HEIGHT, 0}};
    static Point BOTTOM_WALL_VEC = {0,1,0};


    static Line WALLS[4] = {LEFT_WALL, TOP_WALL, RIGHT_WALL, BOTTOM_WALL};
    static Point WALL_VECS[4] = {LEFT_WALL_VEC, TOP_WALL_VEC, RIGHT_WALL_VEC, BOTTOM_WALL_VEC};

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

    static float dist(const Point &p1, const Point &p2)
    {
        return sqrt(
            pow((p1.x - p2.x), 2) +
            pow((p1.y - p2.y), 2));
    }

    static float dotProd(const Point &p1, const Point &p2)
    {
        return (p1.x * p2.x) + (p1.y * p2.y) + (p1.z * p2.z);
    }

    static Point intersectionPt(const Line &l1, const Line &l2)
    {
        // y1 = m1*x + c1
        // y2 = m2*x + c2
        // m1*x + c1 = m2*x + c2
        // m1*x + c1 - m2*x - c2 = 0
        // x(m1 - m2) + c1 - c2 = 0

        // x_intersection = (c2 - c1) / (m1 - m2)
        // y_intersection = m1 * x_intersection + c1 or m2 * x_intersection + c2

        // TODO: CHECK IF LINE IS VERTICAL

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

        float x_intersection = (l2.c - l1.c) / (l1.m - l2.m);
        return {x_intersection, l1.m * x_intersection + l1.c};
    }
}
