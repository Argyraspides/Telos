#pragma once
#include <vector>
#include <memory>
#include <string>
#include "cartesian.h"

enum WALL_SIDE
{
    LEFT = 0,
    TOP = 1,
    RIGHT = 2,
    BOTTOM = 3,

    NONE = 4,
    TOP_BOTTOM = 5,
    LEFT_RIGHT = 6
};

enum SHAPE_TYPE_IDENTIFIERS
{
    POINT_CLOUD_SHAPE = 0,
    AABB = 1,
    BOUNDING_CIRCLE = 2
};

enum BODY_TYPE_IDENTIFIERS
{
    RIGID_BODY = 0,
    SOFT_BODY = 1,
    PARTICLE = 2,
    FLUID = 3
};

class Shape
{
public:
    Shape(int shapeTypeID, int bodyTypeID);

    // THE TYPE OF SHAPE THAT IT IS E.G. POINT CLOUD (WHICH IS ASSUMED BY DEFAULT)
    virtual const int getShapeTypeID() const
    {
        return this->m_shapeTypeID;
    }

    // THE ID OF THE ACTUAL SHAPE IN THE WORLD E.G. SHAPE #1, SHAPE #2 ...
    virtual const long long getShapeID() const
    {
        return this->m_shapeID;
    }

    virtual void moveShape(const Point &p)
    {
    }

    virtual void updateShape(const double &timeStep, const int &timeDir)
    {
    }

    virtual void setShapePos(const Point &p)
    {
    }

    virtual void setShapeVel(const Point &p)
    {
    }

    virtual void rotShape(const double &rad, const Point &pivot)
    {
    }

    virtual ~Shape()
    {
    }

public:
    static long long ID_CTR; // NEXT AVAILABLE ID OF SHAPE
    int m_shapeTypeID;       // TYPE OF SHAPE (E.G. RIGID BODY, SOFT BODY)
    int m_bodyTypeID;        // TYPE OF BODY (E.G. RIGID BODY)
    long long m_shapeID;     // UNIQUE IDENTIFIER FOR AN INDIVIDUAL SHAPE
    double m_rot;            // ROTATION (RADIANS)
    double m_rotInert;       // ROTATIONAL INERTIA
    Point m_center;          // CENTER OF SHAPE
    Point m_vel;             // VELOCITY
    double m_mass;           // MASS
    double m_timeSpawned;    // TIME THE SHAPE WAS CREATED
    std::string m_name;     // NAME OF THE SHAPE E.G. TRIANGLE, CIRCLE, PARTICLE
};