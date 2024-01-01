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

    void UpdateModel_AddShape(std::shared_ptr<Shape> shape, Point offset); // ADDS A SHAPE TO THE WORLD
    void UpdateModel_RemoveShape(std::shared_ptr<Shape> shape);            // REMOVES A SHAPE FROM THE WORLD
    void UpdateModel_RemoveShape(long long shapeID);                       // REMOVES A SHAPE FROM THE WORLD BASED ON ITS ID
    void UpdateModel_ForwardTick();
    void UpdateModel_BackwardTick();

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
};