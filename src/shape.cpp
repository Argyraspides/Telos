#include "shape.h"
#include "engine_math.h"

long long Shape::ID_CTR = 0;

Shape::Shape(int shapeTypeID, int bodyTypeID)
{
    this->m_shapeTypeID = shapeTypeID;
    this->m_bodyTypeID = bodyTypeID;
    this->m_shapeID = (ID_CTR++);
    this->m_vel = {0, 0, 0};
    this->m_rot = 0;
    this->m_mass = 1.0f;
}

