#pragma once
#include "shape.h"
#include <memory>

class Model
{
public:
    Model();

public:
    void run();

    std::vector<std::shared_ptr<Shape>> m_shapeList;
    std::vector<PointCloudShape_Cvx> m_PCSCVX_shapeList;

    int m_shapeCount;
    bool m_isRunning;
    SHAPE_TYPE_IDENTIFIERS m_shapeType;

    void update(std::vector<PointCloudShape_Cvx> &shapeList);
    Point isContact(PointCloudShape_Cvx s1, PointCloudShape_Cvx s2);
};
