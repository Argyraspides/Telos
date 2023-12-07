#pragma once
#include "shape.h"
#include <memory>

class Model
{
public:
    Model();

public:
    int getShapeCount();
    void addShape(std::shared_ptr<Shape> shape);
    void removeShape(std::shared_ptr<Shape> shape);
    void removeShape(long long shapeID);
    std::vector<std::shared_ptr<Shape>> getShapeList();
    void run();

private:
    std::vector<std::shared_ptr<Shape>> m_shapeList;
    std::vector<PointCloudShape_Cvx> m_PCSCVX_shapeList;
    int m_shapeCount;

// FOR POINT CLOUD SHAPES
private: 
    bool m_isRunning;
    SHAPE_TYPE_IDENTIFIERS m_shapeType;

    long long isContact(std::shared_ptr<Shape> s1, std::shared_ptr<Shape> s2);
};
