#pragma once
#include <vector>
#include <memory>
#include "cartesian.h"

enum SHAPE_TYPE_IDENTIFIERS
{
    // Convex Point Cloud
    POINT_CLOUD_SHAPE_CVX = 0,
    // Arbitrary Point Cloud (concave or convex)
    POINT_CLOUD_SHAPE_ARB = 1,
    // Axis-Aligned Bounding Box
    AABB = 2
};

enum BODY_TYPE_IDENTIFIERS
{
    RIGID_BODY = 0,
    SOFT_BODY = 1,
    PARTICLE = 2,
    FLUID = 3
};

class Shape
{
public:
    Shape(int shapeTypeID, int bodyTypeID);

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

    virtual void moveShape(const Point &p)
    {
    }

    virtual void moveAndRotShape()
    {
    }

    virtual void setShapePos(const Point &p)
    {
    }

    virtual void rotShape(const float &rad, const Point &pivot)
    {
    }

    virtual ~Shape()
    {
    }

public:
    static long long ID_CTR; // NEXT AVAILABLE ID OF SHAPE
    int m_shapeTypeID;       // TYPE OF SHAPE (E.G. POINT CLOUD)
    int m_bodyTypeID;        // TYPE OF BODY (E.G. RIGID BODY)
    long long m_shapeID;     // UNIQUE IDENTIFIER FOR AN INDIVIDUAL SHAPE
    float m_rot;             // ROTATION (RADIANS)
    Point m_center;          // CENTER OF SHAPE
    Point m_vel;             // VELOCITY
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
    std::vector<Point> getPoints() const; // RETURNS POINT CLOUD
    void setPoint(const Point &p, const int &pointNum);
    int getShapeTypeID() const override { return SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX; } // RETURNS THE TYPE OF SHAPE

    void moveShape(const Point &p) override;
    void moveAndRotShape() override;
    void setShapePos(const Point &p) override;
    void rotShape(const float &rad, const Point &pivot) override;

public:
    static std::vector<Point> generateCircle(float radius);                   // CONSTRUCTS A CIRCLE USING RADIUS
    static std::vector<Point> generateRectangle(float w, float h);            // CONSTRUCTS A RECTANGLE USING WIDTH AND HEIGHT
    static std::vector<Point> generateTriangle(Point p1, Point p2, Point p3); // CONSTRUCTS A TRIANGLE USING THREE POINTS
    static std::vector<Point> generateTriangle(float t1, float t2, float t3); // CONSTRUCTS A TRIANGLE USING THREE ANGLES FROM THE X AXIS OF A UNIT CIRCLE
    static const int CIRCLE_POINT_COUNT = 15;

public:
    std::vector<Point> m_points; // POINT CLOUD THAT REPRESENTS THE SHAPE
    std::vector<Point> m_initPoints;
    std::vector<float> m_pointsRadial; // REPRESENTS THE DISTANCE OF EACH POINT FROM THE SHAPES CENTER
};

// ***************************************************************************************************************************************************************
// Utilities class for various operations on shapes, e.g. converting a unique shape data structure into the principle point cloud,
// calculating the centroid of various shapes, checking if a shape is convex, etc
class ShapeUtils
{
public:
    static std::vector<Point> convertToPointCloud(const std::shared_ptr<Shape> &shape); // CONVERTS ANY SHAPE DATA STRUCTURE INTO A POINT CLOUD SHAPE
    static Point getCentroid(const std::vector<Point> &points);                         // CALCULATES CENTROID OF THE SHAPE
    static bool checkConvex(const std::vector<Point> &points);                          // CHECKS IF A SHAPE IS CONVEX
    static bool isInside(Point p, const std::shared_ptr<Shape> &shape);                 // CHECKS IF A POINT IS INSIDE OF A CONVEX POINT CLOUD
    static void printAllShapeInfo(PointCloudShape_Cvx s);                               // PRINTS SHAPE POINT COORDINATES, CENTER, VELOCITY, ROTATION, MASS
    static void printLineInfo(Line l);
    static void printPointInfo(Point p);
};
// ***************************************************************************************************************************************************************