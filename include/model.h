#pragma once
#include "shape.h"
#include <memory>

class Model
{
public:
    Model();

public:
    void addShape(std::shared_ptr<Shape> shape);
    void removeShape(std::shared_ptr<Shape> shape);
    void removeShape(long long shapeID);
    void run();

    int getShapeCount();

    std::vector<std::shared_ptr<Shape>> getShapeList();

private:
    std::vector<std::shared_ptr<Shape>> m_shapeList;
    std::vector<PointCloudShape_Cvx> m_PCSCVX_shapeList;
    int m_shapeCount;

    // FOR POINT CLOUD SHAPES
private:
    bool m_isRunning;
    SHAPE_TYPE_IDENTIFIERS m_shapeType;

    void update(std::vector<PointCloudShape_Cvx> &shapeList);

    // Detects collision between two convex point cloud shapes. Returns the point of collision
    Point isContact(PointCloudShape_Cvx s1, PointCloudShape_Cvx s2);
};
