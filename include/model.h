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
    // to the run() function so we can actually pass it into pthread_create()
    static void *threadEntry(void *instance)
    {
        reinterpret_cast<Model *>(instance)->run();
        return nullptr;
    }

public:
    void run();

    std::vector<std::shared_ptr<Shape>> m_shapeList;
    std::vector<std::shared_ptr<Shape>> &getShapeList();
    pthread_mutex_t shapeListMutex;

    std::vector<std::shared_ptr<PointCloudShape_Cvx>> m_PCSCVX_shapeList;
    std::vector<std::shared_ptr<PointCloudShape_Cvx>> &getPCSCVXShapeList();
    pthread_mutex_t PCSCVXShapeListMutex;

    bool m_isRunning;
    SHAPE_TYPE_IDENTIFIERS m_shapeType;

    void updatePCSL();
    Point isContact(PointCloudShape_Cvx s1, PointCloudShape_Cvx s2);
};
