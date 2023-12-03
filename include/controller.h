#pragma once
#include "imgui.h"
#include "shape.h"

class Controller
{

public:

    Controller();

    void addPointCloudShape(const std::vector<Point> &points);
    void removeShape();
    void changeShape();
    void changeGravity();
    void changeWallElasticity();

    void CommonShapeSubMenu();
    void CircleButton();
    void FullMenu();

    std::vector<Point> ResolveShapeDefinition(const Shape &shape);

    void setCurrentShapeType(int shapeTypeID);
};