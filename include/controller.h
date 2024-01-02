#pragma once
#include "imgui.h"
#include "shape.h"
#include "model.h"

struct MODEL_MODIFICATION_STATUS
{
    int currentStatus;

    const int PCS_ADD_FAIL_INSUFFICIENT_POINTS = -1;
    const int PCS_ADD_SUCCESS = 0;

    bool failed()
    {
        return (currentStatus < 0);
    }
};

class Controller
{

private:
    Model *m_model;

public:
    Controller(Model *model);

    void UpdateModel_AddShape(std::shared_ptr<Shape> shape, Point offset); // ADDS A SHAPE TO THE WORLD
    void UpdateModel_AddShape_RegularPoly(double radius, int sides, double xVel, double yVel, double rot, double mass);
    void UpdateModel_AddShape_Rect(double w, double h, double xVel, double yVel, double rot, double mass);
    MODEL_MODIFICATION_STATUS UpdateModel_AddShape_Arbitrary(char pts[], double xVel, double yVel, double rot, double mass);
    void UpdateModel_RemoveShape(std::shared_ptr<Shape> shape);            // REMOVES A SHAPE FROM THE WORLD
    void UpdateModel_RemoveShape(long long shapeID);                       // REMOVES A SHAPE FROM THE WORLD BASED ON ITS ID
    void UpdateModel_RemoveAllShapes();                       // REMOVES A SHAPE FROM THE WORLD BASED ON ITS ID
    void UpdateModel_ForwardTick();
    void UpdateModel_BackwardTick();
    void UpdateModel_ChangeElasticity(double e);
    void UpdateModel_ChangeWallElasticity(double e);

    void ShutModel();         // SHUTS OFF THE MODEL
    void PauseUnpauseModel(); // FLIPS THE MODELS PAUSE STATE
    void PauseModel();
    void UnpauseModel();

    const std::vector<std::shared_ptr<Shape>> &RetrieveModel_ReadShapes(); // RETRIEVES LIST OF SHAPES FROM THE MODEL AS READ-ONLY
    const int RetrieveModel_GetShapeCount();                               // RETURNS NUMBER OF SHAPES FROM THE MODEL
    const Point RetrieveModel_GetMaxVelocity();
    const double RetrieveModel_GetMaxRotVelocity();
    const double RetrieveModel_GetTimeStep();
    const double RetrieveModel_GetCurrentTime();
    const double RetrieveModel_GetMaxEnergyViolation();
    const int RetrieveModel_GetMaxPCSPoints();
    const double RetrieveModel_GetMinElasticity();
    const double RetrieveModel_GetMaxElasticity();
    const int RetrieveModel_GetMaxObjects();
};