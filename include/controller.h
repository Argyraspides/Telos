#pragma once
#include "imgui.h"
#include "shape.h"
#include "model.h"

class Controller
{

private:
    Model *model;


public:

    Controller(Model* model);

    void addPointCloudShape(std::vector<Point> points, Point offset);
    void moveShape(int shapeID);
    void removeShape();
    void changeShape();
    void changeGravity();
    void changeWallElasticity();



    std::vector<Point> ResolveShapeDefinition(std::shared_ptr<Shape> shape);

    void setCurrentShapeType(int shapeTypeID);

};