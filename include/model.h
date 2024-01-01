#pragma once
#include "shape.h"
#include "point_cloud_convex.h"
#include <mutex>
#include <memory>

// Information required to resolve collisions between convex point cloud polygons
struct CollisionInfo_PCSCVX
{
    CollisionInfo_PCSCVX(bool hasCollided)
    {
        this->hasCollided = hasCollided;
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
        this->hasCollided = hasCollided;
        this->collisionPoint = collisionPoint;
        this->penetrationVector = penetrationVector;
        this->penetrationLine = penetrationLine;
        this->collisionSurfaceNormal = collisionSurfaceNormal;
        this->penetrationDepth = penetrationDepth;

        this->s1 = s1;
        this->s2 = s2;
    }

    bool hasCollided;
    PointCloudShape_Cvx *s1; // The two shapes that have collided. The first one is always the penetrator, the second penetrated.
    PointCloudShape_Cvx *s2;
    Point collisionPoint;
    Point penetrationVector; // Penetration vector is always given in the direction of the penetrating shape to the penetrated one
    Line penetrationLine;
    Point collisionSurfaceNormal; // Describes the surface line that the collision point contacted
    double penetrationDepth;
};

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

enum TIME_DIRECTION {
    STILL = 0,
    FORWARD = 1,
    BACKWARD = -1
};

struct WallCollisionInfo_PCSCVX
{

    WallCollisionInfo_PCSCVX(bool collided, int wallSide, PointCloudShape_Cvx *shape, Point collisionPoint)
    {
        this->collided = collided;
        this->wallSide = wallSide;
        this->shape = shape;
        this->collisionPoint = collisionPoint;
    }

    WallCollisionInfo_PCSCVX(bool collided)
    {
        this->collided = false;
    }

    bool collided = false;
    // Which side of the wall it collided with
    int wallSide;
    // The shape that collided with the wall
    PointCloudShape_Cvx *shape;
    // Point of collision
    Point collisionPoint;
};

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

    void updatePCSL(int timeDirection); // UPDATES THE POINT CLOUD SHAPE LIST (PCSL)

    bool isContactPCSCVX_SAT(PointCloudShape_Cvx &s1, PointCloudShape_Cvx &s2);                // DETERMINES IF TWO SHAPES OF TYPE POINT CLOUD HAVE COLLIDED
    CollisionInfo_PCSCVX isContactPCSCVX_CL(PointCloudShape_Cvx &s1, PointCloudShape_Cvx &s2); // DETERMINES IF TWO SHAPES OF TYPE POINT CLOUD HAVE COLLIDED AND RETURNS COLLISION INFORMATION

    WallCollisionInfo_PCSCVX isContactWallLinear(PointCloudShape_Cvx &s1); // DETERMINES IF A POINT CLOUD SHAPE HAS COLLIDED WITH THE WALL
    WallCollisionInfo_PCSCVX isContactWallLinearRot(PointCloudShape_Cvx &s1);
    std::vector<std::pair<PointCloudShape_Cvx &, PointCloudShape_Cvx &>> isContactBroad(); // BROAD PHASE COLLISION DETECTION. QUICKLY FILTERS OUT SHAPES THAT "OBVIOUSLY" WILL NOT COLLIDE

    void resolveCollisionPCSCVX(CollisionInfo_PCSCVX &collisionInfo);                               // RESOLVES COLLISION BETWEEN TWO POINT CLOUD SHAPES
    void resolveCollisionOverlapPCSCVX(CollisionInfo_PCSCVX &collisionInfo);                        // RESOLVES COLLISION BETWEEN TWO POINT CLOUD SHAPES (OVERLAP SEPARATION)
    void resolveOverlapCollisionPCSCVX_Wall_Linear(WallCollisionInfo_PCSCVX &wallCollisionInfo);    // RESOLVES COLLISION BETWEEN POINT CLOUD SHAPE AND THE WALL (OVERLAP SEPARATION ONLY, ONLY TAKES INTO ACCOUNT TRANSLATION)
    void resolveOverlapCollisionPCSCVX_Wall_LinearRot(WallCollisionInfo_PCSCVX &wallCollisionInfo); // RESOLVES COLLISION BETWEEN POINT CLOUD SHAPE AND THE WALL (OVERLAP SEPARATION ONLY, TAKES INTO ACCOUNT ROTATION)
    void resolveCollisionPCSCVX_Wall(WallCollisionInfo_PCSCVX &wallCollisionInfo);                  // RESOLVES COLLISION BETWEEN POINT CLOUD SHAPE AND THE WALL (NORMAL RESOLUTION)
public:
    double m_time = 0.0;
    double m_timeStep = 1.0 / ENGINE_POLLING_RATE;
    const double m_ENERGY_THRESHOLD = 0.0000000001; // CONSERVATION OF ENERGY CAN BE VIOLATED BY THIS MUCH AS PER THE NATURE OF DOUBLE CALCULATIONS
    const double m_SEPARATION_SAFETY_FACTOR = 1.0;

    Point m_maxVel = {7000.0, 7000.0};
    const double m_rotMax = 10.0f;
};
