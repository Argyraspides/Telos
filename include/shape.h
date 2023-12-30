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

    virtual void updateShape(const double &timeStep, const int &timeDir)
    {
    }

    virtual void setShapePos(const Point &p)
    {
    }

    virtual void rotShape(const double &rad, const Point &pivot)
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
    double m_rot;             // ROTATION (RADIANS)
    double m_rotInert;        // ROTATIONAL INERTIA
    Point m_center;          // CENTER OF SHAPE
    Point m_vel;             // VELOCITY
    double m_mass;            // MASS
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
    double getEkrot(); // GETS THE ROTATIONAL KINETIC ENERGY OF THE SHAPE
    double getEk(); // GETS THE KINETIC ENERGY OF THE SHAPE
    double getE(); // GETS THE TOTAL KINETIC ENERGY OF THE SHAPE

    void moveShape(const Point &p) override;
    void updateShape(const double &timeStep, const int &timeDir) override;
    void setShapePos(const Point &p) override;
    void rotShape(const double &rad, const Point &pivot) override;

public:
    static std::vector<Point> generateRegularPolygon(double radius, int sides);                   // CONSTRUCTS A CIRCLE USING RADIUS
    static std::vector<Point> generateRectangle(double w, double h);            // CONSTRUCTS A RECTANGLE USING WIDTH AND HEIGHT
    static std::vector<Point> generateTriangle(Point p1, Point p2, Point p3); // CONSTRUCTS A TRIANGLE USING THREE POINTS
    static std::vector<Point> generateTriangle(double t1, double t2, double t3); // CONSTRUCTS A TRIANGLE USING THREE ANGLES FROM THE X AXIS OF A UNIT CIRCLE
    static const int CIRCLE_POINT_COUNT = 15;

public:
    std::vector<Point> m_points; // POINT CLOUD THAT REPRESENTS THE SHAPE
    std::vector<Point> m_initPoints;
    std::vector<double> m_pointsRadial; // REPRESENTS THE DISTANCE OF EACH POINT FROM THE SHAPES CENTER
    std::vector<Point> m_Deltas;       // GIVES THE X AND Y DISTANCE OF EACH POINT FROM THE SHAPES CENTER
    Point m_initPos;                   // REPRESENTS THE INITIAL POSITION OF THE SHAPE'S CENTER
    double m_time;
};

// ***************************************************************************************************************************************************************
// Utilities class for various operations on shapes, e.g. converting a unique shape data structure into the principle point cloud,
// calculating the centroid of various shapes, checking if a shape is convex, etc
class ShapeUtils
{
public:
    static std::vector<Point> convertToPointCloud(const std::shared_ptr<Shape> &shape); // CONVERTS ANY SHAPE DATA STRUCTURE INTO A POINT CLOUD SHAPE
    static Point getCentroid(const std::vector<Point> &points);                         // CALCULATES CENTROID OF THE SHAPE
    static double getRotInertia(const std::vector<Point> &points);                       // CALCULATES THE ROTATIONAL INTERTIA OF THE SHAPE
    static bool checkConvex(const std::vector<Point> &points);                          // CHECKS IF A SHAPE IS CONVEX
    static bool isInside(Point p, const std::shared_ptr<Shape> &shape);                 // CHECKS IF A POINT IS INSIDE OF A CONVEX POINT CLOUD
    static void printAllShapeInfo(PointCloudShape_Cvx s);                               // PRINTS SHAPE POINT COORDINATES, CENTER, VELOCITY, ROTATION, MASS
    static void printLineInfo(Line l);
    static void printPointInfo(Point p);
};
// ***************************************************************************************************************************************************************