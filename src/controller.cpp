#include "controller.h"
#include "view.h"
#include "model.h"
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>

Controller::Controller(Model *model)
{
    this->model = model;
}

void Controller::UpdateModel_AddPointCloudShape(std::vector<Point> points, Point offset)
{
    for (Point &p : points)
        p = p + offset;

    PointCloudShape_Cvx pcs(points);
    std::shared_ptr<Shape> shape = std::make_shared<PointCloudShape_Cvx>(pcs);
    this->model->addShape(shape);
}

void Controller::UpdateModel_MoveShape(int shapeID)
{
}
