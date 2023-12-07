#include "model.h"
Model::Model()
{
    this->m_shapeCount = 0;
    this->m_shapeType = SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX;
}

// TODO: TEST
void Model::addShape(std::shared_ptr<Shape> shape)
{
    this->m_shapeList.push_back(shape);
    if(shape->getShapeTypeID() == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        std::shared_ptr<PointCloudShape_Cvx> pointCloudShape_Cvx = std::dynamic_pointer_cast<PointCloudShape_Cvx>(shape);
        this->m_PCSCVX_shapeList.push_back(*pointCloudShape_Cvx);
    }
    else if(shape->getShapeTypeID() == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_ARB)
    {

    }
    m_shapeCount++;
}

// TODO: IMPLEMENT
void Model::removeShape(std::shared_ptr<Shape> shape)
{

}

// TODO: IMPLEMENT
void Model::removeShape(long long shapeID)
{
    
}

std::vector<std::shared_ptr<Shape>> Model::getShapeList()
{
    return this->m_shapeList;
}

int Model::getShapeCount()
{
    return this->m_shapeCount;
}


void Model::run()
{
    while(m_isRunning)
    {
        if(this->m_shapeType == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
        {
            while(m_isRunning)
            {

            }
        }

        else if(this->m_shapeType == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_ARB)
        {
            while(m_isRunning)
            {

            }
        }
    }
}