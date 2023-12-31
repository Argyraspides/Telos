#include "shape_utils.h"
#include <iostream>

// For any shape, resolves what kind of shape it is and then translates it into a point cloud for rendering
std::vector<Point> ShapeUtils::convertToPointCloud(const std::shared_ptr<Shape> &shape)
{
    int shapeTypeID = shape->getShapeTypeID();
    if (shapeTypeID == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        // Point cloud shapes are already of type point cloud. Just return points.
        std::shared_ptr<PointCloudShape_Cvx> pointCloudShape_Cvx = std::dynamic_pointer_cast<PointCloudShape_Cvx>(shape);
        return pointCloudShape_Cvx->getPoints();
    }
    std::cerr << "SHAPE TYPE IS INVALID. ERROR IN FUNCTION: " << __func__ << " IN CLASS " << typeid(ShapeUtils).name() << std::endl;
    return {};
}

// Checks if a point lies within a shape
bool ShapeUtils::isInside(Point p, const std::shared_ptr<Shape> &s)
{

    int shapeTypeID = s->getShapeTypeID();
    if (shapeTypeID == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {

        // // Treating the point as the origin of a cartesian plane, if the polygon takes up all four quadrants, the point lies within the polygon

        // std::shared_ptr<PointCloudShape_Cvx> pointCloudShape_Cvx = std::dynamic_pointer_cast<PointCloudShape_Cvx>(s);

        // int quadrantCount[4] = {0, 0, 0, 0};

        // std::vector<Point> shapePoints = pointCloudShape_Cvx->getPoints();

        // for (int i = 0; i < shapePoints.size(); i++)
        // {
        //     // Quadrants 1, 2, 3, and 4 (not in order)

        //     double sx = shapePoints[i].x;
        //     double sy = shapePoints[i].y;

        //     if (sx < p.x && sy < p.y)
        //     {
        //         quadrantCount[0]++;
        //     }
        //     else if (sx < p.x && sy > p.y)
        //     {
        //         quadrantCount[1]++;
        //     }
        //     else if (sx > p.x && sy < p.y)
        //     {
        //         quadrantCount[2]++;
        //     }
        //     else if (sx > p.x && sy > p.y)
        //     {
        //         quadrantCount[3]++;
        //     }
        // }

        // // If the points cover all four quadrants, the point is inside the polygon.
        // bool isInside = quadrantCount[0] > 0 && quadrantCount[1] > 0 && quadrantCount[2] > 0 && quadrantCount[3] > 0;
        // std::cout << isInside << "\n";
        // return isInside ;

        std::shared_ptr<PointCloudShape_Cvx> pointCloudShape_Cvx = std::dynamic_pointer_cast<PointCloudShape_Cvx>(s);
        std::vector<Point> shapePoints = pointCloudShape_Cvx->getPoints();

        bool inside = false;
        for (int i = 0; i < shapePoints.size(); ++i)
        {
            const Point &v1 = shapePoints[i];
            const Point &v2 = shapePoints[(i + 1) % shapePoints.size()];

            if ((v1.y > p.y) != (v2.y > p.y) &&
                p.x < (v2.x - v1.x) * (p.y - v1.y) / (v2.y - v1.y) + v1.x)
            {
                inside = !inside;
            }
        }

        return inside;
    }

    std::cerr << "SHAPE TYPE IS INVALID. ERROR IN FUNCTION: " << __func__ << " IN CLASS " << typeid(ShapeUtils).name() << std::endl;
    return false;
}

// Calculates the centroid of a polygon given its vertices (assumes that the polygon has an even mass distribution)
Point ShapeUtils::getCentroid(const std::vector<Point> &points)
{
    // Assuming an even mass distribution the centroid is simply the average location of all the points
    double sumX = 0.0, sumY = 0.0;
    for (const Point &p : points)
    {
        sumX += p.x;
        sumY += p.y;
    }
    return Point(sumX / (double)points.size(), sumY / (double)points.size());
}

// TODO: IMPLEMENT
bool ShapeUtils::checkConvex(const std::vector<Point> &points)
{
    for (size_t i = 0; i < points.size(); i++)
    {
        
    }

    return true;
}

void ShapeUtils::printAllShapeInfo(PointCloudShape_Cvx s)
{
    std::cout << "polygon(";
    for (int i = 0; i < s.m_points.size(); i++)
    {
        std::cout << "(" << s.m_points[i].x << "," << s.m_points[i].y << ")";
        if (i < s.m_points.size() - 1)
            std::cout << ",";
    }
    std::cout << ")\n";

    // std::cout //<< "CENTER: "
    //     << "(" << s.m_center.x << "," << s.m_center.y << ")\n";

    // std::cout << "VELOCITY: (" << s.m_vel.x << "," << s.m_vel.y << ")\n";

    // std::cout << "\n";
}

void ShapeUtils::printLineInfo(Line l)
{
    if (!l.isVertical)
        std::cout << "y = " << l.m << "x + " << l.c << "\n";
    else
        std::cout << "x = " << l.x << "\n";
}

void ShapeUtils::printPointInfo(Point p)
{
    std::cout << "(" << p.x << "," << p.y << ")\n";
}

// Obtain the rotational inertia, "I", of an arbitrary polygon
double ShapeUtils::getRotInertia(const std::vector<Point> &points)
{

    double j_x = 0, j_y = 0;

    for (int v = 0; v < points.size() - 1; v++)
    {
        int vpp = v + 1;
        // (x_i * y_i+1 - x_i+1 * y_i)
        double leftTerm =
            points[v].x * points[vpp].y - points[vpp].x * points[v].y;

        // (y_i^2 + y_i * y_i+1 + y_i+1^2)
        double rightTerm =
            pow(points[v].y, 2) + points[v].y * points[vpp].y + pow(points[vpp].y, 2);

        j_x += leftTerm + rightTerm;
        j_y += leftTerm;

        // (x_i^2 + x_i * x_i+1 + x_i+1^2)
        rightTerm =
            pow(points[v].x, 2) + points[v].x * points[vpp].x + pow(points[vpp].x, 2);

        j_y += rightTerm;
    }

    double oneTwelfth = 1.0f / 12.0f;
    j_x *= oneTwelfth;
    j_y *= oneTwelfth;

    return (j_x + j_y);

}

