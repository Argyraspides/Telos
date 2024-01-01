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
    m_model = model;
}

void Controller::UpdateModel_AddShape(std::shared_ptr<Shape> shape, Point offset)
{
    // Determine the type of shape it is
    int id = shape->getShapeTypeID();
    if (id == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        // Convert to point cloud shape
        std::shared_ptr<PointCloudShape_Cvx> pointCloudShape_Cvx = std::dynamic_pointer_cast<PointCloudShape_Cvx>(shape);
        // Offset the shape
        pointCloudShape_Cvx->moveShape(offset);

        // Add to the models point cloud shape convex (PCSVX) list and generic shape list
        m_model->getPCSCVXShapeList().push_back(pointCloudShape_Cvx);
        m_model->getShapeList().push_back(shape);

        // getShapeList() and getPCSCVXShapeList() lock the models mutex's for obtaining shape data. We have to unlock them here.
        pthread_mutex_unlock(&m_model->shapeListMutex);
        pthread_mutex_unlock(&m_model->PCSCVXShapeListMutex);
    }
    else if (id == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_ARB)
    {
    }
}

void Controller::UpdateModel_RemoveShape(std::shared_ptr<Shape> shape)
{
    // Determine the type of shape it is
    int ShapeTypeID = shape->getShapeTypeID();
    long long shapeID = shape->getShapeID();
    if (ShapeTypeID == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        std::vector<std::shared_ptr<Shape>> &shapeList = m_model->getShapeList();
        // PCSCVX means "Point Cloud Shape Convex"
        std::vector<std::shared_ptr<PointCloudShape_Cvx>> &PCSCVXList = m_model->getPCSCVXShapeList();

        // Free the pointers to the shapes and erase them from the vector
        shapeList[shapeID].reset();
        PCSCVXList[shapeID].reset();

        PCSCVXList.erase(PCSCVXList.begin() + shapeID);
        shapeList.erase(shapeList.begin() + shapeID);

        // Shift all shape ID's down by one ahead of the deleted one
        for (size_t i = shapeID; i < shapeList.size(); i++)
        {
            shapeList[i]->m_shapeID--;
        }
        // Next available shape ID is now one less than before
        Shape::ID_CTR--;

        // getShapeList() and getPCSCVXShapeList() lock the models mutex's for obtaining shape data. We have to unlock them here.
        pthread_mutex_unlock(&m_model->shapeListMutex);
        pthread_mutex_unlock(&m_model->PCSCVXShapeListMutex);
    }
    else if (ShapeTypeID == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_ARB)
    {
    }
}

// TODO: IMPLEMENT
void Controller::UpdateModel_RemoveShape(long long shapeID)
{
}

void Controller::UpdateModel_ForwardTick()
{
    if (m_model->m_isPaused)
    {
        m_model->updatePCSL(TIME_DIRECTION::FORWARD);
    }
}

void Controller::UpdateModel_BackwardTick()
{
    if (m_model->m_isPaused)
    {
        m_model->updatePCSL(TIME_DIRECTION::BACKWARD);
    }
}

void Controller::UpdateModel_ChangeElasticity(double e)
{
    m_model->m_collisionElasticity = e;
}

const std::vector<std::shared_ptr<Shape>> &Controller::RetrieveModel_ReadShapes()
{
    // We don't need to lock the mutex as this is read-only
    return m_model->m_shapeList;
}

const int Controller::RetrieveModel_GetShapeCount()
{
    // We don't need to lock the mutex as this is read-only
    return m_model->m_shapeList.size();
}

const Point Controller::RetrieveModel_GetMaxVelocity()
{
    return m_model->m_maxVel;
}

const double Controller::RetrieveModel_GetMaxRotVelocity()
{
    return m_model->m_rotMax;
}

const double Controller::RetrieveModel_GetTimeStep()
{
    return m_model->m_timeStep;
}

const double Controller::RetrieveModel_GetCurrentTime()
{
    return m_model->m_time;
}

const double Controller::RetrieveModel_GetMaxEnergyViolation()
{
    return m_model->m_ENERGY_THRESHOLD;
}

const int Controller::RetrieveModel_GetMaxPCSPoints()
{
    return m_model->m_maxPCSPoints;
}

void Controller::ShutModel()
{
    m_model->m_isRunning = false;
}

void Controller::PauseUnpauseModel()
{
    m_model->m_isPaused = !m_model->m_isPaused;
}

void Controller::PauseModel()
{
    m_model->m_isPaused = true;
}

void Controller::UnpauseModel()
{
    m_model->m_isPaused = false;
}