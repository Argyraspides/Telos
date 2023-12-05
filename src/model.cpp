#include "model.h"

Model::Model()
{
    this->shapeCount = 0;
}

void Model::addShape(std::shared_ptr<Shape> shape)
{
    this->shapeList.push_back(shape);
    shapeCount++;
}


std::vector<std::shared_ptr<Shape>> Model::getShapeList()
{
    return this->shapeList;
}


int Model::getShapeCount()
{
    return this->shapeCount;
}