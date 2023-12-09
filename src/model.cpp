#include "model.h"
#include <iostream>

Model::Model()
{
    this->m_shapeCount = 0;
    this->m_shapeType = SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX;
}

void Model::run()
{

    if (this->m_shapeType == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        while (m_isRunning)
        {
            update(this->m_PCSCVX_shapeList);
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
void Model::update(std::vector<PointCloudShape_Cvx> &shapeList)
{
    // Translate
    for (PointCloudShape_Cvx &shape : this->m_PCSCVX_shapeList)
    {
    }

    // Rotate
    for (PointCloudShape_Cvx &shape : this->m_PCSCVX_shapeList)
    {
    }
}

