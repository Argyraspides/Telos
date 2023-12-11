#pragma once
#include "shape.h"
#include <mutex>
#include <memory>

class Model
{
public:
    Model();
    ~Model();

    // Emscripten doesn't support std::thread for multithreading, only C-type pthread's. This will essentially be a pointer
    // to the run() function so we can actually pass it into pthread_create() in main.cpp
    static void *threadEntry(void *instance)
    {
        reinterpret_cast<Model *>(instance)->run();
        return nullptr;
    }

public:
    void run(); // BEGINS THE PHYSICS ENGINE PROCESSING

    std::vector<std::shared_ptr<Shape>> m_shapeList;     // LIST OF SHAPES GIVEN AS GENERIC SHAPES AS AN INTERFACE FOR COMMON CONTROLS
    std::vector<std::shared_ptr<Shape>> &getShapeList(); // RETURNS THE LIST OF SHAPES AS GENERIC SHAPES
    pthread_mutex_t shapeListMutex;                      // MUTEX TO LOCK READ/WRITE ACCESS TO THE SHAPES. THIS IS LOCKED IN getShapeList(), AND MUST BE UNLOCKED AFTER CALLING THE FUNCTION

    std::vector<std::shared_ptr<PointCloudShape_Cvx>> m_PCSCVX_shapeList;
    std::vector<std::shared_ptr<PointCloudShape_Cvx>> &getPCSCVXShapeList();
    pthread_mutex_t PCSCVXShapeListMutex;

    bool m_isRunning;                   // ENGINE LOOP CONDITIONAL
    bool m_isPaused;                    // ENGINE PAUSE CONDITIONAL
    SHAPE_TYPE_IDENTIFIERS m_shapeType; // CURRENT TYPE OF SHAPE THAT THE ENGINE IS DEALING WITH, E.G. POINT CLOUD SHAPES

    void updatePCSL();                                               // UPDATES THE POINT CLOUD SHAPE LIST (PCSL)
    Point isContact(PointCloudShape_Cvx s1, PointCloudShape_Cvx s2); // DETERMINES IF TWO SHAPES OF TYPE POINT CLOUD HAVE COLLIDED
};
