#pragma once
#include "imgui.h"
#include "shape.h"
#include "model.h"

struct MODEL_MODIFICATION_RESULT
{
    int currentStatus;

    /********************POINT CLOUD SHAPE ERRORS*****************/
    /*************************************************************/
    static const int PCS_ADD_FAIL_INSUFFICIENT_POINTS = -1;
    static const int PCS_ADD_FAIL_EXCEEDED_MAX_POINTS = -2;
    static const int PCS_ADD_FAIL_EXCEEDED_MAX_SHAPE_PARAMS = -3;
    static const int PCS_ADD_FAIL_INVALID_POINT_INPUT = -4;
    /*************************************************************/
    static const int PCS_ADD_SUCCESS = 0;
    /*************************************************************/

    MODEL_MODIFICATION_RESULT(int res)
    {
        currentStatus = res;
    }

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

    //*************************************************** RIDID BODY MECHANICS FUNCTIONS ******************************************************
    // ****************************************************************************************************************************************

    void UpdateModel_AddShape(std::shared_ptr<Shape> shape, Point offset); // ADDS A SHAPE TO THE WORLD
    MODEL_MODIFICATION_RESULT UpdateModel_AddShape_RegularPoly(double radius, int sides, double xVel, double yVel, double rot, double mass);
    MODEL_MODIFICATION_RESULT UpdateModel_AddShape_Rect(double w, double h, double xVel, double yVel, double rot, double mass);
    MODEL_MODIFICATION_RESULT UpdateModel_AddShape_Arbitrary(char pts[], double xVel, double yVel, double rot, double mass);

    void UpdateModel_RemoveShape(std::shared_ptr<Shape> shape); // REMOVES A SHAPE FROM THE WORLD
    void UpdateModel_RemoveShape(long long shapeID);            // REMOVES A SHAPE FROM THE WORLD BASED ON ITS ID
    void UpdateModel_RemoveAllShapes();                         // REMOVES A SHAPE FROM THE WORLD BASED ON ITS ID

    void UpdateModel_ForwardTick();  // MOVES THE ENGINE FORWARD BY ONE TIME STEP
    void UpdateModel_BackwardTick(); // MOVES THE ENGINE BACKWARD BY ONE TIME STEP

    void UpdateModel_ChangeElasticity(double e);     // CHANGES THE ELASTICITY OF OBJECT-OBJECT COLLISIONS
    void UpdateModel_ChangeWallElasticity(double e); // CHANGES THE ELASTICITY OF OBJECT-WALL COLLISIONS

    void ShutModel(); // SHUTS OFF THE MODEL

    void PauseUnpauseModel(); // FLIPS THE MODELS PAUSE STATE
    void PauseModel();        // PAUSES THE MODEL OUTRIGHT
    void UnpauseModel();      // UNPAUSES THE MODEL OUTRIGHT

    const std::vector<std::shared_ptr<Shape>> &RetrieveModel_ReadShapes(); // RETRIEVES LIST OF SHAPES FROM THE MODEL AS READ-ONLY
    const int RetrieveModel_GetShapeCount();                               // RETURNS NUMBER OF SHAPES FROM THE MODEL
    const Point RetrieveModel_GetMaxVelocity();                            // GET THE MAXIMUM ALLOWED VELOCITY OF AN OBJECT
    const double RetrieveModel_GetMaxRotVelocity();                        // GET THE MAXIMUM ALLOWED ROTATIONAL VELOCITY OF AN OBJECT (RADIANS/S)
    const double RetrieveModel_GetTimeStep();                              // GET THE ENGINE TIME STEP (dt)
    const double RetrieveModel_GetCurrentTime();                           // GET THE TOTAL TIME ELAPSED SINCE THE ENGINE BEGAN
    const double RetrieveModel_GetMaxEnergyViolation();                    // GET CONSERVATION OF ENERGY THRESHOLD
    const int RetrieveModel_GetMaxPCSPoints();                             // GET THE MAXIMUM ALLOWED POINTS OF A CONVEX POLYGON
    const double RetrieveModel_GetMinElasticity();                         // GET THE MINIMUM ALLOWED COLLISION ELASTICITY
    const double RetrieveModel_GetMaxElasticity();                         // GET THE MAXIMUM ALLOWED COLLISION ELASTICITY
    const int RetrieveModel_GetMaxObjects();                               // GET THE MAXIMUM ALLOWED OBJECTS THAT THE ENGINE CAN HANDLE
    const double RetrieveModel_GetMaxMass();                                  // GET THE MAXIMUM ALLOWED MASS OF AN OBJECT
    // ****************************************************************************************************************************************
};