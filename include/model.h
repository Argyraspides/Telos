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
    std::vector<std::shared_ptr<Shape>> getShapeList();

private:
    std::vector<std::shared_ptr<Shape>> shapeList;
    int shapeCount;
};
