#pragma once
#include <vector>
#include <memory>
#include "cartesian.h"

static long long ID_CTR = 0;

enum SHAPE_TYPE_IDENTIFIERS
{
    // Convex Point Cloud
    POINT_CLOUD_SHAPE_CVX = 0,
    // Arbitrary Point Cloud (concave or convex)
    POINT_CLOUD_SHAPE_ARB = 1,
    // Axis-Aligned Bounding Box
    AABB = 2
};

class Shape
{
public:
    Shape(int shapeTypeID);

    // THE TYPE OF SHAPE THAT IT IS E.G. POINT CLOUD (WHICH IS ASSUMED BY DEFAULT)
    virtual int getShapeTypeID() const
    {
        return this->m_shapeTypeID;
    }

    // THE ID OF THE ACTUAL SHAPE IN THE WORLD E.G. SHAPE #1, SHAPE #2 ...
    virtual long long getShapeID() const
    {
        return this->m_shapeID;
    }

    virtual Point getCenter() const
    {
        return this->m_center;
    }

    // MOVE THE SHAPE
    virtual void moveShape(const Point &p)
    {
    }

    virtual void setShapePos(const Point &p)
    {
    }

    virtual ~Shape() {}

protected:
    long long m_shapeID;
    int m_shapeTypeID;
    Point m_center;
    float m_xVel; // X-AXIS VELOCITY
    float m_yVel; // Y-AXIS VELOCITY
    float m_rot;  // ROTATION (RADIANS)
};

// ***************************************************************************************************************************************************************
// A point cloud is a data structure that represents a shape as nothing more than a set of points. Curvature is approximated with many points.
// "_Cvx" means a convex shape.

class PointCloudShape_Cvx : public Shape
{

public:
    PointCloudShape_Cvx(const std::vector<Point> &points);
    PointCloudShape_Cvx();

public:
    std::vector<Point> getPoints() const;                                                         // RETURNS POINT CLOUD
    int getShapeTypeID() const override { return SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX; } // RETURNS THE TYPE OF SHAPE

public:
    void moveShape(const Point &p) override;
    void setShapePos(const Point &p) override;

public:
    static std::vector<Point> generateCircle(float radius);                   // CONSTRUCTS A CIRCLE USING RADIUS
    static std::vector<Point> generateRectangle(float w, float h);            // CONSTRUCTS A RECTANGLE USING WIDTH AND HEIGHT
    static std::vector<Point> generateTriangle(Point p1, Point p2, Point p3); // CONSTRUCTS A TRIANGLE USING THREE POINTS
    static std::vector<Point> generateTriangle(float t1, float t2, float t3); // CONSTRUCTS A TRIANGLE USING THREE ANGLES FROM THE X AXIS OF A UNIT CIRCLE
    static const int CIRCLE_POINT_COUNT = 15;

private:
    std::vector<Point> m_points; // POINT CLOUD THAT REPRESENTS THE SHAPE
};

// ***************************************************************************************************************************************************************
// Utilities class for various operations on shapes, e.g. converting a unique shape data structure into the principle point cloud,
// calculating the centroid of various shapes,
class ShapeUtils
{
public:
    static std::vector<Point> convertToPointCloud(std::shared_ptr<Shape> shape); // CONVERTS ANY SHAPE DATA STRUCTURE INTO A POINT CLOUD SHAPE
    static Point getCentroid(const std::vector<Point> &points);                  // CALCULATES CENTROID OF THE SHAPE
    static bool checkConvex(const std::vector<Point> &points);                   // CHECKS IF A SHAPE IS CONVEX
    static bool isInside(Point p, std::shared_ptr<Shape> s);                     // CHECKS IF A POINT IS INSIDE OF A CONVEX POINT CLOUD
    static void printInfo(PointCloudShape_Cvx s);                                // PRINTS SHAPE POINT COORDINATES, CENTER, VELOCITY, ROTATION, MASS
};
// ***************************************************************************************************************************************************************