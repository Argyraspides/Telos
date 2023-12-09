#pragma once
#include "imgui.h"
#include "shape.h"
#include "model.h"

class Controller
{

private:
    Model *model;

public:
    Controller(Model *model);

    long long UpdateModel_AddPointCloudShape(PointCloudShape_Cvx s, Point offset);
    void UpdateModel_MoveShape(long long shapeID);
    void UpdateModel_AddShape(std::shared_ptr<Shape> shape);
    void UpdateModel_RemoveShape(std::shared_ptr<Shape> shape);
    void UpdateModel_RemoveShape(long long shapeID);

    const std::vector<std::shared_ptr<Shape>>& RetrieveModel_GetShapes();
    int RetrieveModel_GetShapeCount();
};