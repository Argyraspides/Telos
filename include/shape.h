#pragma once

#include <vector>

#define SHAPE_CIRCLE 0
#define SHAPE_SQUARE 1
#define SHAPE_RECTANGLE 2

#define CIRCLE_POINT_COUNT 15

// ***************************************************************************************************************************************************************
// This shape is represented as nothing more than a set of points, known as a "point cloud". Curvature is approximated with many points.

struct Point {
    int x = 0, y = 0, z = 0;
};

class PointCloudShape_Cvx
{
public:
    PointCloudShape_Cvx(const std::vector<Point> &points);      // CONSTRUCTS THE SHAPE 
    std::vector<Point> getPoints();                         // RETURNS ORDERED POINTS OF SHAPE

private:
    std::vector<Point> m_points;
};

// ***************************************************************************************************************************************************************