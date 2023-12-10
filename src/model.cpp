#include "model.h"
#include <chrono>
#include <iostream>

#define POLLING_RATE_S 60 // POLLING RATE (POLLS PER SECOND)

Model::Model()
{
    this->m_shapeCount = 0;
    this->m_shapeType = SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX;
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
                update(this->m_PCSCVX_shapeList);
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

// TODO: UPDATE POSITIONS VIA POINTS
void Model::update(std::vector<std::shared_ptr<PointCloudShape_Cvx>> shapeList)
{
    // Translate
    for (int i = 0; i < this->m_PCSCVX_shapeList.size(); i++)
    {
        shapeList[i]->moveShape(shapeList[i]->m_vel);
    }

    // Rotate
    // for (PointCloudShape_Cvx &shape : this->m_PCSCVX_shapeList)
    // {
    //     shape.rotShape(shape.m_rot, shape.m_center);
    // }
}
