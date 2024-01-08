#pragma once
#include "cartesian.h"
#define NULLPOINT Point(-1,-1,-1)
struct ModelEvent
{
    Point collisionLocation = NULLPOINT;
};
