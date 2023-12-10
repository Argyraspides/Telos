#include "model.h"
#include <chrono>
#include <iostream>

#define POLLING_RATE_S 60 // POLLING RATE (POLLS PER SECOND)

Model::Model()
{
    this->m_shapeType = SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX;
    
    pthread_mutex_init(&this->shapeListMutex, nullptr);
    pthread_mutex_init(&this->PCSCVXShapeListMutex, nullptr);
}

Model::~Model()
{
    pthread_mutex_destroy(&this->shapeListMutex);
    pthread_mutex_destroy(&this->PCSCVXShapeListMutex);
}

void Model::run()
{
    this->m_isRunning = true;
    if (this->m_shapeType == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        const std::chrono::milliseconds frameDuration(1000 / POLLING_RATE_S);
        auto startTime = std::chrono::high_resolution_clock::now();

        while (m_isRunning)
        {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

            if (elapsed >= frameDuration)
            {
                updatePCSL();
                startTime = std::chrono::high_resolution_clock::now();
            }
        }
    }

    else if (this->m_shapeType == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_ARB)
    {
        while (m_isRunning)
        {
        }
    }
}
// Update Point Cloud Shape List
void Model::updatePCSL()
{
    // Translate
    size_t size = this->getPCSCVXShapeList().size();
    for (size_t i = 0; i < size; i++)
    {
        this->m_PCSCVX_shapeList[i]->moveShape(m_PCSCVX_shapeList[i]->m_vel);
    }
    pthread_mutex_unlock(&PCSCVXShapeListMutex);
}

// MAKE SURE TO CALL pthread_mutex_unlock() ONCE YOU ARE DONE ACCESSING THE LIST
std::vector<std::shared_ptr<Shape>> &Model::getShapeList()
{
    pthread_mutex_lock(&shapeListMutex);
    return this->m_shapeList;
}
// MAKE SURE TO CALL pthread_mutex_unlock() ONCE YOU ARE DONE ACCESSING THE LIST
std::vector<std::shared_ptr<PointCloudShape_Cvx>> &Model::getPCSCVXShapeList()
{
    pthread_mutex_lock(&PCSCVXShapeListMutex);
    return this->m_PCSCVX_shapeList;
}
