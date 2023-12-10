#include "controller.h"
#include "view.h"
#include "model.h"
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>

Controller::Controller(Model *model)
{
    this->model = model;
}

long long Controller::UpdateModel_AddPointCloudShape(PointCloudShape_Cvx s, Point offset)
{
    s.m_center = s.m_center + offset;
    for (Point &p : s.m_points)
        p = p + offset;

    std::shared_ptr<Shape> shape = std::make_shared<PointCloudShape_Cvx>(s);
    UpdateModel_AddShape(shape);
    return shape->getShapeID();
}

void Controller::UpdateModel_MoveShape(long long shapeID)
{
}

void Controller::UpdateModel_AddShape(std::shared_ptr<Shape> shape)
{
    if (shape->getShapeTypeID() == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        std::shared_ptr<PointCloudShape_Cvx> pointCloudShape_Cvx = std::dynamic_pointer_cast<PointCloudShape_Cvx>(shape);

        this->model->getPCSCVXShapeList().push_back(pointCloudShape_Cvx);
        this->model->getShapeList().push_back(shape);

        pthread_mutex_unlock(&this->model->shapeListMutex);
        pthread_mutex_unlock(&this->model->PCSCVXShapeListMutex);
    }
    else if (shape->getShapeTypeID() == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_ARB)
    {
    }
}

void Controller::UpdateModel_RemoveShape(std::shared_ptr<Shape> shape)
{
    int id = shape->getShapeID();
    if (shape->getShapeTypeID() == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        std::vector<std::shared_ptr<Shape>> &shapeList = this->model->getShapeList();
        std::vector<std::shared_ptr<PointCloudShape_Cvx>> &PCSCVXList = this->model->getPCSCVXShapeList();

        shapeList[id].reset();
        PCSCVXList[id].reset();

        PCSCVXList.erase(PCSCVXList.begin() + id);
        shapeList.erase(shapeList.begin() + id);

        for (size_t i = id; i < shapeList.size(); i++)
        {
            shapeList[i]->m_shapeID--;
        }
        Shape::ID_CTR--;
        pthread_mutex_unlock(&this->model->shapeListMutex);
        pthread_mutex_unlock(&this->model->PCSCVXShapeListMutex);
    }
    else if (shape->getShapeTypeID() == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_ARB)
    {
    }
}

// TODO: IMPLEMENT
void Controller::UpdateModel_RemoveShape(long long shapeID)
{
}

const std::vector<std::shared_ptr<Shape>> &Controller::RetrieveModel_ReadShapes()
{
    std::vector<std::shared_ptr<Shape>> &vec = this->model->getShapeList();
    pthread_mutex_unlock(&this->model->shapeListMutex);
    return vec;
}

const int Controller::RetrieveModel_GetShapeCount()
{
    std::vector<std::shared_ptr<Shape>> &vec = this->model->getShapeList();
    pthread_mutex_unlock(&this->model->shapeListMutex);
    return vec.size();
}

void Controller::ShutModel()
{
    this->model->m_isRunning = false;
}