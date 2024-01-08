#pragma once
#include "shape.h"
#include "point_cloud_convex.h"
#include "telos_events.h"
#include "engine_math.h"
#include <mutex>
#include <memory>


// Information required to resolve collisions between convex point cloud polygons
struct CollisionInfo_PCSCVX
{
    CollisionInfo_PCSCVX(bool hasCollided)
    {
        this->m_collided = hasCollided;
    }

    CollisionInfo_PCSCVX()
    {
        
    }

    CollisionInfo_PCSCVX(
        bool hasCollided,
        Point collisionPoint,
        Point penetrationVector,
        Line penetrationLine,
        Point collisionSurfaceNormal,
        double penetrationDepth,
        PointCloudShape_Cvx *s1,
        PointCloudShape_Cvx *s2)
    {
        this->m_collided = hasCollided;
        this->m_collisionPoint = collisionPoint;
        this->m_penetrationVector = penetrationVector;
        this->m_penetrationLine = penetrationLine;
        this->m_collisionSurfaceNormal = collisionSurfaceNormal;
        this->m_penetrationDepth = penetrationDepth;

        this->s1 = s1;
        this->s2 = s2;
    }

    bool m_collided;
    PointCloudShape_Cvx *s1; // The two shapes that have collided. The first one is always the penetrator, the second penetrated.
    PointCloudShape_Cvx *s2;
    Point m_collisionPoint;
    Point m_penetrationVector; // Penetration vector is always given in the direction of the penetrating shape to the penetrated one
    Line m_penetrationLine;
    Point m_collisionSurfaceNormal; // Describes the surface line that the collision point contacted
    double m_penetrationDepth;
};

enum TIME_DIRECTION
{
    STILL = 0,
    FORWARD = 1,
    BACKWARD = -1
};

struct WallCollisionInfo_PCSCVX
{

    WallCollisionInfo_PCSCVX(bool collided, bool multiWall, int wallSide, PointCloudShape_Cvx *shape, Point collisionPoint, Point secondCollisionPoint = {0, 0, 0}, int secondWallside = -1)
    {
        this->m_collided = collided;
        this->m_multiWall = multiWall;
        this->m_wallSide = wallSide;
        this->m_shape = shape;
        this->m_collisionPoint = collisionPoint;
        this->m_secondCollisionPoint = secondCollisionPoint;
        this->m_secondWallSide = secondWallside;
        setCollisionNormal();
    }

    WallCollisionInfo_PCSCVX(bool collided)
    {
        this->m_collided = false;
    }

    void setCollisionNormal()
    {
        if (m_multiWall)
        {
            Point colsVec = m_collisionPoint - m_secondCollisionPoint;
            m_collisionNormal = Math::getNormal2D(colsVec);

            bool bl = (m_wallSide == WALL_SIDE::BOTTOM && m_secondWallSide == WALL_SIDE::LEFT) || (m_wallSide == WALL_SIDE::LEFT && m_secondWallSide == WALL_SIDE::BOTTOM);
            bool tl = (m_wallSide == WALL_SIDE::TOP && m_secondWallSide == WALL_SIDE::LEFT) || (m_wallSide == WALL_SIDE::LEFT && m_secondWallSide == WALL_SIDE::TOP);
            bool tr = (m_wallSide == WALL_SIDE::TOP && m_secondWallSide == WALL_SIDE::RIGHT) || (m_wallSide == WALL_SIDE::RIGHT && m_secondWallSide == WALL_SIDE::TOP);

            if (bl)
            {
                m_collisionNormal.x = abs(m_collisionNormal.x);
                m_collisionNormal.y = -abs(m_collisionNormal.y);
            }
            else if (tl)
            {
                m_collisionNormal.x = abs(m_collisionNormal.x);
                m_collisionNormal.y = abs(m_collisionNormal.y);
            }
            else if (tr)
            {
                m_collisionNormal.x = -abs(m_collisionNormal.x);
                m_collisionNormal.y = abs(m_collisionNormal.y);
            }
            else
            {
                m_collisionNormal.x = -abs(m_collisionNormal.x);
                m_collisionNormal.y = -abs(m_collisionNormal.y);
            }

            m_collisionNormal.normalize();
        }
        else
        {
            m_collisionNormal = Math::WALL_VECS[m_wallSide];
        }
    }

    bool m_collided = false;
    bool m_multiWall = false;
    // Which side of the wall it collided with
    // Optional: if the shape collided with two sides of a wall simultaneously

    int m_wallSide;
    int m_secondWallSide;
    // The shape that collided with the wall
    PointCloudShape_Cvx *m_shape;
    // Point of collision
    Point m_collisionPoint;
    Point m_secondCollisionPoint;
    Point m_collisionNormal;
};


class Model
{
public:
    Model();
    ~Model();
    void run(); // BEGINS THE PHYSICS ENGINE PROCESSING

    // Emscripten doesn't support std::thread for multithreading, only C-type pthread's. This will essentially be a pointer
    // to the run() function so we can actually pass it into pthread_create() in main.cpp
    static void *threadEntry(void *instance)
    {
        reinterpret_cast<Model *>(instance)->run();
        return nullptr;
    }

public:
    std::vector<std::shared_ptr<Shape>> m_shapeList;     // LIST OF SHAPES GIVEN AS GENERIC SHAPES AS AN INTERFACE FOR COMMON CONTROLS
    std::vector<std::shared_ptr<Shape>> &getShapeList(); // RETURNS THE LIST OF SHAPES AS GENERIC SHAPES
    pthread_mutex_t shapeListMutex;                      // MUTEX TO LOCK READ/WRITE ACCESS TO THE SHAPES. THIS IS LOCKED IN getShapeList(), AND MUST BE UNLOCKED AFTER CALLING THE FUNCTION

public:
    //*************************************************** RIDID BODY MECHANICS FUNCTIONS ******************************************************
    // ****************************************************************************************************************************************

    std::vector<ModelEvent> m_modelEvents;

    std::vector<std::shared_ptr<PointCloudShape_Cvx>> m_PCSCVX_shapeList;
    std::vector<std::shared_ptr<PointCloudShape_Cvx>> &getPCSCVXShapeList();
    pthread_mutex_t PCSCVXShapeListMutex;

    bool m_isRunning;                   // ENGINE LOOP CONDITIONAL
    bool m_isPaused;                    // ENGINE PAUSE CONDITIONAL
    SHAPE_TYPE_IDENTIFIERS m_shapeType; // CURRENT TYPE OF SHAPE THAT THE ENGINE IS DEALING WITH, E.G. POINT CLOUD SHAPES
    BODY_TYPE_IDENTIFIERS m_bodyType;

    void updatePCSCVXList(int timeDirection); // UPDATES THE POINT CLOUD CONVEX SHAPE SLIST

    bool isContactPCSCVX_SAT(PointCloudShape_Cvx &s1, PointCloudShape_Cvx &s2);                // DETERMINES IF TWO SHAPES OF TYPE POINT CLOUD HAVE COLLIDED
    CollisionInfo_PCSCVX isContactPCSCVX_CL(PointCloudShape_Cvx &s1, PointCloudShape_Cvx &s2); // DETERMINES IF TWO SHAPES OF TYPE POINT CLOUD HAVE COLLIDED AND RETURNS COLLISION INFORMATION

    WallCollisionInfo_PCSCVX isContactWall(PointCloudShape_Cvx &s1); // DETERMINES IF A POINT CLOUD SHAPE HAS COLLIDED WITH THE WALL
    bool QuickIsContactWall(PointCloudShape_Cvx &s1);
    std::vector<std::pair<PointCloudShape_Cvx &, PointCloudShape_Cvx &>> isContactBroadPCSVX(); // BROAD PHASE COLLISION DETECTION. QUICKLY FILTERS OUT SHAPES THAT "OBVIOUSLY" WILL NOT COLLIDE

    void resolveCollisionPCSCVX(CollisionInfo_PCSCVX &collisionInfo);                         // RESOLVES COLLISION BETWEEN TWO POINT CLOUD SHAPES
    void resolveCollisionOverlapPCSCVX(CollisionInfo_PCSCVX &collisionInfo);                  // RESOLVES COLLISION BETWEEN TWO POINT CLOUD SHAPES (OVERLAP SEPARATION)
    void resolveCollisionOverlapPCSCVX_Rot(PointCloudShape_Cvx &s1, PointCloudShape_Cvx &s2); // RESOLVES COLLISION BETWEEN TWO CONVEX POINT CLOUD SHAPES (OVERLAP SEPARATION) TAKING INTO ACCOUNT BOTH ROTATION AND TRANSLATION

    void resolveCollisionPCSCVX_Wall(WallCollisionInfo_PCSCVX &wci);        // RESOLVES COLLISION BETWEEN POINT CLOUD SHAPE AND THE WALL (NORMAL RESOLUTION)
    void resolveCollisionOverlapPCSCVX_Wall(WallCollisionInfo_PCSCVX &wci); // RESOLVES COLLISION BETWEEN POINT CLOUD SHAPE AND THE WALL (OVERLAP SEPARATION ONLY, ONLY TAKES INTO ACCOUNT TRANSLATION)
    void resolveCollisionOverlapPCSCVX_Wall_Rot(PointCloudShape_Cvx &s1);   // RESOLVES COLLISION BETWEEN POINT CLOUD SHAPE AND THE WALL (OVERLAP SEPARATION ONLY, TAKES INTO ACCOUNT TRANSLATION AND ROTATION)

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:
    const double m_ENERGY_THRESHOLD = 0.0000000001; // CONSERVATION OF ENERGY CAN BE VIOLATED BY THIS MUCH AS PER THE NATURE OF DOUBLE CALCULATIONS
    const double m_SEPARATION_SAFETY_FACTOR = 1.0;
    double m_collisionElasticity = 1.0;
    double m_wallCollisionElasticity = 1.0;
    int m_wallOverlapResolution = 10;
    int m_shapeOverlapResolution = 10;

    const Point m_maxVel = {2500.0, 2500.0};
    const double m_maxRot = 10.0f;
    const int m_maxPCSPoints = 100;
    const int m_maxObjects = 50;
    const double m_maxMass = 200.0;

    const double m_minCollisionElasticity = 0.0;
    const double m_maxCollisionElasticity = 1.0;

    const int m_minWallOverlapResolution = 0;
    const int m_maxWallOverlapResolution = 20;

    const int m_minShapeOverlapResolution = 0;
    const int m_maxShapeOverlapResolution = 15;

    double m_time = 0.0;
    double m_timeStep = 1.0 / ENGINE_POLLING_RATE;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    typedef void (Model::*CurrentUpdateFunc)(int);
    CurrentUpdateFunc m_currentUpdateFunc; // Specifies the current function used to update the engine
};
