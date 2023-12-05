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

    void addPointCloudShape(const std::vector<Point> &points);
    void moveShape(int shapeID);
    void removeShape();
    void changeShape();
    void changeGravity();
    void changeWallElasticity();
    void CommonShapeSubMenu();
    void CircleButton();
    void FullMenu();

    std::vector<Point> ResolveShapeDefinition(std::shared_ptr<Shape> shape);

    void setCurrentShapeType(int shapeTypeID);

};