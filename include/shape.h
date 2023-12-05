#pragma once
#include <vector>
static int ID_CTR = 0;

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
        return this->shapeTypeID;
    }

    // THE ID OF THE ACTUAL SHAPE IN THE WORLD E.G. SHAPE #1, SHAPE #2 ...
    virtual int getShapeID()
    {
        return this->shapeID;
    }

    virtual ~Shape() {}

protected:
    int shapeID;
    int shapeTypeID;
};

// ***************************************************************************************************************************************************************
// A point cloud is a data structure that represents a shape as nothing more than a set of points. Curvature is approximated with many points.
// "_Cvx" means a convex shape.

struct Point
{
    float x, y, z;
    Point(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    Point operator+(const Point& point)
    {
        return {
            x + point.x,
            y + point.y,
            z + point.z
        };
    }
};

class PointCloudShape_Cvx : public Shape
{
public:
    PointCloudShape_Cvx(const std::vector<Point> &points); // CONSTRUCTS THE SHAPE
    PointCloudShape_Cvx();                                 // DEFAULT CONSTRUCTOR
    std::vector<Point> getPoints() const;                  // RETURNS POINT CLOUD
    int getShapeTypeID() const override { return POINT_CLOUD_SHAPE_CVX; }

public:
    // COMMON SHAPES
    static std::vector<Point> generateCircle(float radius);                   // CONSTRUCTS A CIRCLE USING RADIUS
    static std::vector<Point> generateRectangle(float w, float h);            // CONSTRUCTS A RECTANGLE USING WIDTH AND HEIGHT
    static std::vector<Point> generateTriangle(Point p1, Point p2, Point p3); // CONSTRUCTS A TRIANGLE USING THREE POINTS
    static std::vector<Point> generateTriangle(float t1, float t2, float t3); // CONSTRUCTS A TRIANGLE USING THREE ANGLES FROM THE X AXIS OF A UNIT CIRCLE
    static const int CIRCLE_POINT_COUNT = 15;

private:
    std::vector<Point> m_points;                        // POINT CLOUD THAT REPRESENTS THE SHAPE
    bool checkConvex(const std::vector<Point> &points); // CHECKS IF A SHAPE IS CONVEX
    Point center;                                       // REPRESENTS THE CENTROID OF THE SHAPE
};

// ***************************************************************************************************************************************************************