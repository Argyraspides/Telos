#include "controller.h"
#include "shape_utils.h"
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

    if (m_model->m_shapeList.size() >= m_model->m_maxObjects)
        return;

    // Determine the type of shape it is
    int id = shape->getShapeTypeID();
    if (id == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE)
    {
        // Convert to point cloud shape
        std::shared_ptr<PointCloudShape_Cvx> pointCloudShape_Cvx = std::dynamic_pointer_cast<PointCloudShape_Cvx>(shape);
        // Offset the shape
        pointCloudShape_Cvx->moveShape(offset);

        // Add to the models point cloud shape convex (PCSVX) list and generic shape list
        m_model->getPCSCVXShapeList().push_back(pointCloudShape_Cvx);
        m_model->getShapeList().push_back(shape);

        // getShapeList() and getPCSCVXShapeList() lock the models mutex's for obtaining shape data. We have to unlock them here.
        pthread_mutex_unlock(&m_model->PCSCVXShapeListMutex);
    }

    pthread_mutex_unlock(&m_model->shapeListMutex);
}

MODEL_MODIFICATION_RESULT Controller::UpdateModel_AddShape_RegularPoly(double radius, int sides, double xVel, double yVel, double rot, double mass)
{
    if (sides < 3)
    {
        return MODEL_MODIFICATION_RESULT(MODEL_MODIFICATION_RESULT::PCS_ADD_FAIL_INSUFFICIENT_POINTS);
    }
    if (sides > m_model->m_maxPCSPoints)
    {
        return MODEL_MODIFICATION_RESULT(MODEL_MODIFICATION_RESULT::PCS_ADD_FAIL_EXCEEDED_MAX_POINTS);
    }
    if (abs(xVel) > abs(m_model->m_maxVel.x) || abs(yVel) > abs(m_model->m_maxVel.y) || abs(rot) > abs(m_model->m_rotMax) || mass <= 0 || abs(mass) > abs(m_model->m_maxMass))
    {
        return MODEL_MODIFICATION_RESULT(MODEL_MODIFICATION_RESULT::PCS_ADD_FAIL_EXCEEDED_MAX_SHAPE_PARAMS);
    }

    // Engine polls at 30-60 times per second. Input values should be intuitive to the user and hence
    // on the order of once per second, so we divide the values by the engines polling rate.
    // E.g. instead of x velocity being 8 pixels every 20ms, its 8 pixels every second.
    PointCloudShape_Cvx regularPoly(Utils::generateRegularPolygon(radius, sides), {xVel / ENGINE_POLLING_RATE, yVel / ENGINE_POLLING_RATE}, rot / ENGINE_POLLING_RATE, mass, RetrieveModel_GetCurrentTime());
    std::shared_ptr<Shape> polyGeneric = std::make_shared<PointCloudShape_Cvx>(regularPoly);
    UpdateModel_AddShape(polyGeneric, {SCREEN_WIDTH / 2.0F, SCREEN_HEIGHT / 2.0F});

    return MODEL_MODIFICATION_RESULT(MODEL_MODIFICATION_RESULT::PCS_ADD_SUCCESS);
}

MODEL_MODIFICATION_RESULT Controller::UpdateModel_AddShape_Rect(double w, double h, double xVel, double yVel, double rot, double mass)
{
    if (abs(xVel) > abs(m_model->m_maxVel.x) || abs(yVel) > abs(m_model->m_maxVel.y) || abs(rot) > abs(m_model->m_rotMax) || mass <= 0 || abs(mass) > abs(m_model->m_maxMass))
    {
        return MODEL_MODIFICATION_RESULT(MODEL_MODIFICATION_RESULT::PCS_ADD_FAIL_EXCEEDED_MAX_SHAPE_PARAMS);
    }

    PointCloudShape_Cvx Rectangle(Utils::generateRectangle(w, h), {xVel / ENGINE_POLLING_RATE, yVel / ENGINE_POLLING_RATE}, rot / ENGINE_POLLING_RATE, mass, RetrieveModel_GetCurrentTime());
    std::shared_ptr<Shape> RectangleGeneric = std::make_shared<PointCloudShape_Cvx>(Rectangle);
    UpdateModel_AddShape(RectangleGeneric, {SCREEN_WIDTH / 2.0F, SCREEN_HEIGHT / 2.0F});

    return MODEL_MODIFICATION_RESULT(MODEL_MODIFICATION_RESULT::PCS_ADD_SUCCESS);
}

MODEL_MODIFICATION_RESULT Controller::UpdateModel_AddShape_Arbitrary(char ptsPtr[], double xVel, double yVel, double rot, double mass)
{

    if (abs(xVel) > abs(m_model->m_maxVel.x) || abs(yVel) > abs(m_model->m_maxVel.y) || abs(rot) > abs(m_model->m_rotMax) || mass <= 0 || abs(mass) > abs(m_model->m_maxMass))
    {
        return MODEL_MODIFICATION_RESULT(MODEL_MODIFICATION_RESULT::PCS_ADD_FAIL_EXCEEDED_MAX_SHAPE_PARAMS);
    }

    std::vector<Point> pts = Utils::generateArbPoly2D(std::string(ptsPtr));
    if (pts.size() >= 3 && pts.size() <= RetrieveModel_GetMaxPCSPoints())
    {
        PointCloudShape_Cvx arbPoly(pts, {xVel / ENGINE_POLLING_RATE, yVel / ENGINE_POLLING_RATE}, rot / ENGINE_POLLING_RATE, mass, RetrieveModel_GetCurrentTime());
        std::shared_ptr<Shape> arbPolyGeneric = std::make_shared<PointCloudShape_Cvx>(arbPoly);
        UpdateModel_AddShape(arbPolyGeneric, {SCREEN_WIDTH / 2.0F, SCREEN_HEIGHT / 2.0F});
    }
    else
    {
        return MODEL_MODIFICATION_RESULT(MODEL_MODIFICATION_RESULT::PCS_ADD_FAIL_INVALID_POINT_INPUT);
    }

    return MODEL_MODIFICATION_RESULT(MODEL_MODIFICATION_RESULT::PCS_ADD_SUCCESS);
}

void Controller::UpdateModel_RemoveShape(std::shared_ptr<Shape> shape)
{
    // Determine the type of shape it is
    int ShapeTypeID = shape->getShapeTypeID();
    long long shapeID = shape->getShapeID();
    if (ShapeTypeID == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE)
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
        pthread_mutex_unlock(&m_model->PCSCVXShapeListMutex);
    }
    pthread_mutex_unlock(&m_model->shapeListMutex);
}

void Controller::UpdateModel_RemoveShape(long long shapeID)
{
}

void Controller::UpdateModel_RemoveAllShapes()
{
    std::vector<std::shared_ptr<Shape>> &shapeList = m_model->getShapeList();
    if (shapeList.size() == 0)
    {
        pthread_mutex_unlock(&m_model->shapeListMutex);
        return;
    }

    int ShapeTypeID = shapeList[0]->getShapeTypeID();
    long long shapeID = shapeList[0]->getShapeID();

    if (ShapeTypeID == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE)
    {
        // PCSCVX means "Point Cloud Shape Convex"
        std::vector<std::shared_ptr<PointCloudShape_Cvx>> &PCSCVXList = m_model->getPCSCVXShapeList();

        // Free the pointers to the shapes and erase them from the vector
        for (size_t i = 0; i < shapeList.size(); i++)
        {
            shapeList[i].reset();
            PCSCVXList[i].reset();
        }
        PCSCVXList.clear();
        shapeList.clear();

        // Next available shape ID is now reset
        Shape::ID_CTR = 0;

        // getShapeList() and getPCSCVXShapeList() lock the models mutex's for obtaining shape data. We have to unlock them here.
        pthread_mutex_unlock(&m_model->PCSCVXShapeListMutex);
    }
    pthread_mutex_unlock(&m_model->shapeListMutex);
}

void Controller::UpdateModel_ForwardTick()
{
    if (m_model->m_isPaused)
    {
        (m_model->*m_model->m_currentUpdateFunc)(TIME_DIRECTION::FORWARD);
    }
}

void Controller::UpdateModel_BackwardTick()
{
    if (m_model->m_isPaused)
    {
        (m_model->*m_model->m_currentUpdateFunc)(TIME_DIRECTION::BACKWARD);
    }
}

void Controller::UpdateModel_ChangeElasticity(double e)
{
    m_model->m_collisionElasticity = e;
}

void Controller::UpdateModel_ChangeWallElasticity(double e)
{
    m_model->m_wallCollisionElasticity = e;
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

const double Controller::RetrieveModel_GetMinElasticity()
{
    return m_model->m_minCollisionElasticity;
}

const double Controller::RetrieveModel_GetMaxElasticity()
{
    return m_model->m_maxCollisionElasticity;
}

const int Controller::RetrieveModel_GetMaxObjects()
{
    return m_model->m_maxObjects;
}

const double Controller::RetrieveModel_GetMaxMass()
{
    return m_model->m_maxMass;
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