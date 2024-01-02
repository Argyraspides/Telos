#include "shape_utils.h"
#include <algorithm>
#include <iostream>


const std::string Utils::shapeNames[] =
    {
        {"Void"},
        {"Point"},
        {"Line Segment"},
        { "Triangle"},
        { "Rectangle"},
        { "Pentagon"},
        { "Hexagon"},
        { "Heptagon"},
        { "Octagon"},
        { "Nonagon"},
        { "Decagon"},
        { "Hendecagon"},
        { "Dodecagon"},
        { "Tridecagon"},
        { "Tetradecagon"},
        { "Pentadecagon"},
        { "Hexadecagon"},
        { "Heptadecagon"},
        { "Octadecagon"},
        { "Enneadecagon"},
        { "Icosagon"},
        { "Icosikaihenagon"},
        { "Icosikaidiagon"},
        { "Icosikaitriagon"},
        { "Icosikaitetragon"},
        { "Icosikaipentagon"},
        { "Icosikaihexagon"},
        { "Icosikaiheptagon"},
        { "Icosikaioctagon"},
        { "Icosikaienneagon"},
        { "Triacontagon"},
        { "Triacontakaihenagon"},
        { "Triacontakaidiagon"},
        { "Triacontakaitriagon"},
        { "Triacontakaitetragon"},
        { "Triacontakaipentagon"},
        { "Triacontakaihexagon"},
        { "Triacontakaiheptagon"},
        { "Triacontakaioctagon"},
        { "Triacontakaienneagon"},
        { "Tetracontagon"},
        { "Tetracontakaihenagon"},
        { "Tetracontakaidiagon"},
        { "Tetracontakaitriagon"},
        { "Tetracontakaitetragon"},
        { "Tetracontakaipentagon"},
        { "Tetracontakaihexagon"},
        { "Tetracontakaiheptagon"},
        { "Tetracontakaioctagon"},
        { "Tetracontakaienneagon"},
        { "Pentacontagon"},
        { "Pentacontakaihenagon"},
        { "Pentacontakaidiagon"},
        { "Pentacontakaitriagon"},
        { "Pentacontakaitetragon"},
        { "Pentacontakaipentagon"},
        { "Pentacontakaihexagon"},
        { "Pentacontakaiheptagon"},
        { "Pentacontakaioctagon"},
        { "Pentacontakaienneagon"},
        { "Hexacontagon"},
        { "Heptacontagon"},
        { "Octacontagon"},
        { "Enneacontagon"},
        { "Tetracontakaitetragon"},
        { "Pentacontakaitetragon"},
        { "Hexacontakaihexagon"},
        { "Heptacontakaiheptagon"},
        { "Octacontakaitoctagon"},
        { "Enneacontakaienneagon"}};

// For any shape, resolves what kind of shape it is and then translates it into a point cloud for rendering
std::vector<Point> Utils::convertToPointCloud(const std::shared_ptr<Shape> &shape)
{
    int shapeTypeID = shape->getShapeTypeID();
    if (shapeTypeID == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        // Point cloud shapes are already of type point cloud. Just return points.
        std::shared_ptr<PointCloudShape_Cvx> pointCloudShape_Cvx = std::dynamic_pointer_cast<PointCloudShape_Cvx>(shape);
        return pointCloudShape_Cvx->getPoints();
    }
    std::cerr << "SHAPE TYPE IS INVALID. ERROR IN FUNCTION: " << __func__ << " IN CLASS " << typeid(Utils).name() << std::endl;
    return {};
}

// Checks if a point lies within a shape
bool Utils::isInside(Point p, const std::shared_ptr<Shape> &s)
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

    std::cerr << "SHAPE TYPE IS INVALID. ERROR IN FUNCTION: " << __func__ << " IN CLASS " << typeid(Utils).name() << std::endl;
    return false;
}

// Calculates the centroid of a polygon given its vertices (assumes that the polygon has an even mass distribution)
Point Utils::getCentroid(const std::vector<Point> &points)
{
    // Assuming an even mass distribution the centroid is simply the average location of all the points
    double sumX = 0.0, sumY = 0.0, sumZ = 0.0;
    for (const Point &p : points)
    {
        sumX += p.x;
        sumY += p.y;
        sumZ += p.z;
    }
    double size = (double)points.size();
    return Point(sumX / size, sumY / size, sumZ / size);
}

// TODO: IMPLEMENT
bool Utils::checkConvex(const std::vector<Point> &points)
{
    for (size_t i = 0; i < points.size(); i++)
    {
    }

    return true;
}

void Utils::printAllShapeInfo(PointCloudShape_Cvx s)
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

void Utils::printLineInfo(Line l)
{
    if (!l.isVertical)
        std::cout << "y = " << l.m << "x + " << l.c << "\n";
    else
        std::cout << "x = " << l.x << "\n";
}

void Utils::printPointInfo(Point p)
{
    std::cout << "(" << p.x << "," << p.y << ")\n";
}

double Utils::getTranslationalKineticEnergy(const Shape &s)
{
    double v = s.m_vel.magnitude();
    double ekTrans = 0.5 * s.m_mass * v * v;
    return ekTrans;
}

double Utils::getRotationalKineticEnergy(const Shape &s)
{
    return 0.5 * s.m_rotInert * s.m_rot * s.m_rot;
}

double Utils::getTotalKineticEnergy(const Shape &s)
{
    return getTranslationalKineticEnergy(s) + getRotationalKineticEnergy(s);
}

// Obtain the rotational inertia, "I", of an arbitrary polygon
double Utils::getRotInertia(const std::vector<Point> &points)
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

std::vector<Point> Utils::generateRegularPolygon(double radius, int sides)
{
    double increment = 2.0 * M_PI / (double)sides;
    std::vector<Point> regularPolygon;
    regularPolygon.reserve(sides);

    double start = -M_PI / 2.0;

    for (int i = 0; i < sides; i++)
    {
        double ang = start + (i * increment);
        regularPolygon.push_back({radius * cos(ang), radius * sin(ang), 0});
    }
    return regularPolygon;
}

std::vector<Point> Utils::generateRectangle(double w, double h)
{
    return {
        {0, 0},
        {w, 0},
        {w, h},
        {0, h}};
}

std::vector<Point> Utils::generateTriangle(Point p1, Point p2, Point p3)
{
    std::vector<Point> pts = {p1, p2, p3};
    std::sort(pts.begin(), pts.end(), [](const Point &a, const Point &b)
              { return a.x < b.x; });
    return pts;
}

std::vector<Point> Utils::generateArbPoly2D(const std::string &s)
{

    // Inputs are defined as:
    // (x,y),(x,y),(x,y) ... to generate an arbitrary polygon

    // A polygon must have at least three points. Minimum amount of characters as per the input
    // rules is 17

    std::vector<Point> fail = {{0, 0, 0}};

    if (s.size() < 17)
        return fail;

    std::vector<Point> pts;

    int i = 0;
    while (i < s.size())
    {
        if (s[i] != '(')
            return fail;
        i++;

        Point inputPt;
        double xy[2] = {0, 0};

        for (int n = 0; n < 2; n++)
        {

            std::string num;

            while (s[i] != ',' && s[i] != ')' && i < s.size())
            {
                num += s[i];
                i++;
            }

            if (num.size() == 0)
            {
                return fail;
            }

            size_t pos;
            xy[n] = std::stod(num, &pos);

            if (pos == 0)
            {
                return fail;
            }

            i++;
        }
        inputPt.x = xy[0];
        inputPt.y = xy[1];
        pts.push_back(inputPt);

        i++;
    }

    return pts;
}