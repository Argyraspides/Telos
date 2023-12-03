#pragma once
#include <vector>

class Shape {
public:
    virtual int getID() const = 0;
    virtual ~Shape() {}
};


// ***************************************************************************************************************************************************************
// ID'S FOR ALL SHAPE DATA STRUCTURES

#define POINT_CLOUD_SHAPE_CVX 0

// ***************************************************************************************************************************************************************
// TODO: LABEL THIS
#define CIRCLE_POINT_COUNT 15

// ***************************************************************************************************************************************************************
// A point cloud is a data structure that represents a shape as nothing more than a set of points. Curvature is approximated with many points.
// "_Cvx" means a convex shape.

struct Point
{
    float x, y, z;
    Point(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {} 
};

class PointCloudShape_Cvx : public Shape
{
public:
    PointCloudShape_Cvx(const std::vector<Point> &points); // CONSTRUCTS THE SHAPE
    PointCloudShape_Cvx();                                 // DEFAULT CONSTRUCTOR
    std::vector<Point> getPoints() const;                  // RETURNS POINT CLOUD
    int getID() const override { return POINT_CLOUD_SHAPE_CVX; }
private:
    std::vector<Point> m_points;                        // POINT CLOUD THAT REPRESENTS THE SHAPE
    bool checkConvex(const std::vector<Point> &points); // CHECKS IF A SHAPE IS CONVEX
    Point center;                                       // REPRESENTS THE CENTROID OF THE SHAPE
};

// ***************************************************************************************************************************************************************