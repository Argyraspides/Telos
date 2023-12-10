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

long long Controller::UpdateModel_AddPointCloudShape(PointCloudShape_Cvx s, Point offset)
{
    s.m_center = s.m_center + offset;
    for(Point &p : s.m_points)
        p = p + offset;

    std::shared_ptr<Shape> shape = std::make_shared<PointCloudShape_Cvx>(s);
    UpdateModel_AddShape(shape);
    return shape->getShapeID();
}

void Controller::UpdateModel_MoveShape(long long shapeID)
{
}

// TODO: TEST
void Controller::UpdateModel_AddShape(std::shared_ptr<Shape> shape)
{
    this->model->m_shapeList.push_back(shape);
    if (shape->getShapeTypeID() == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        std::shared_ptr<PointCloudShape_Cvx> pointCloudShape_Cvx = std::dynamic_pointer_cast<PointCloudShape_Cvx>(shape);
        this->model->m_PCSCVX_shapeList.push_back(pointCloudShape_Cvx);
    }
    else if (shape->getShapeTypeID() == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_ARB)
    {
    }
}

// TODO: TEST
void Controller::UpdateModel_RemoveShape(std::shared_ptr<Shape> shape)
{
    int id = shape->getShapeID();
    if (shape->getShapeTypeID() == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        this->model->m_shapeList[id].reset();
        this->model->m_PCSCVX_shapeList[id].reset();

        this->model->m_PCSCVX_shapeList.erase(this->model->m_PCSCVX_shapeList.begin() + id);
        this->model->m_shapeList.erase(this->model->m_shapeList.begin() + id);

        for(int i = id; i < this->model->m_shapeList.size(); i++)
        {
            this->model->m_shapeList[i]->m_shapeID--;
        }
        Shape::ID_CTR--;
    }
    else if (shape->getShapeTypeID() == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_ARB)
    {
    }
}

// TODO: IMPLEMENT
void Controller::UpdateModel_RemoveShape(long long shapeID)
{
}


std::vector<std::shared_ptr<Shape>> Controller::RetrieveModel_GetShapes()
{
    return this->model->m_shapeList;
}

int Controller::RetrieveModel_GetShapeCount()
{
    return this->model->m_shapeList.size();
}

void Controller::ShutModel()
{
    this->model->m_isRunning = false;
}