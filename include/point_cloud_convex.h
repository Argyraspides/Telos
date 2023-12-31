#pragma once
#include "shape.h"

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
