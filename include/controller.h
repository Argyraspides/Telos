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

    void UpdateModel_AddPointCloudShape(std::vector<Point> points, Point offset);
    void UpdateModel_MoveShape(int shapeID);

};