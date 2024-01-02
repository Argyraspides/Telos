#pragma once
#include "shape.h"
#include <string>
#include "point_cloud_convex.h"
#include <unordered_map>

// Utilities class for various operations on shapes, e.g. converting a unique shape data structure into the principle point cloud,
// calculating the centroid of various shapes, checking if a shape is convex, etc

class Utils
{
public:
    static std::vector<Point> convertToPointCloud(const std::shared_ptr<Shape> &shape); // CONVERTS ANY SHAPE DATA STRUCTURE INTO A POINT CLOUD SHAPE
    static Point getCentroid(const std::vector<Point> &points);                         // CALCULATES CENTROID OF THE SHAPE
    static double getRotInertia(const std::vector<Point> &points);                      // CALCULATES THE ROTATIONAL INTERTIA OF THE SHAPE
    static bool checkConvex(const std::vector<Point> &points);                          // CHECKS IF A SHAPE IS CONVEX
    static bool isInside(Point p, const std::shared_ptr<Shape> &shape);                 // CHECKS IF A POINT IS INSIDE OF A CONVEX POINT CLOUD
    static void printAllShapeInfo(PointCloudShape_Cvx s);                               // PRINTS SHAPE POINT COORDINATES, CENTER, VELOCITY, ROTATION, MASS
    static void printLineInfo(Line l);
    static void printPointInfo(Point p);
    static double getTotalKineticEnergy(const Shape &s);
    static double getTranslationalKineticEnergy(const Shape &s);
    static double getRotationalKineticEnergy(const Shape &s);

public:
    static std::vector<Point> generateRegularPolygon(double radius, int sides);  // CONSTRUCTS A CIRCLE USING RADIUS
    static std::vector<Point> generateRectangle(double w, double h);             // CONSTRUCTS A RECTANGLE USING WIDTH AND HEIGHT
    static std::vector<Point> generateTriangle(Point p1, Point p2, Point p3);    // CONSTRUCTS A TRIANGLE USING THREE POINTS
    static std::vector<Point> generateTriangle(double t1, double t2, double t3); // CONSTRUCTS A TRIANGLE USING THREE ANGLES FROM THE X AXIS OF A UNIT CIRCLE

public:
    static std::vector<Point> generateArbPoly2D(const std::string &s);
    static const int shapeNameCount = 70;
    static const std::string shapeNames[shapeNameCount];
};