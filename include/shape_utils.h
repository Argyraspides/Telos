#pragma once
#include "shape.h"
#include "point_cloud_convex.h"


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