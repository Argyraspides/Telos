#pragma once
#include "cartesian.h"
#include "SDL.h"
#include <vector>
#include <array>

enum ANIMATION_TYPE
{
    PARTICLE_EXPLOSION = 0
};

class Animation
{
public: 
    Animation(double duration);
    virtual void tick(SDL_Renderer* renderer){};
    double duration;
    double timeElapsed;
    
    int animationType;
};

class ParticleExplosionAnimation : public Animation
{
public:
    ParticleExplosionAnimation(Point startPosition, double duration, std::vector<Point> particles, std::vector<Point> particleVelocities);
    ParticleExplosionAnimation(Point startPosition, double duration, int particleCount);
    ParticleExplosionAnimation(Point startPosition, double duration);

    void tick(SDL_Renderer* renderer) override;

    std::vector<Point> particles;
    std::vector<std::array<int, 4>> particleColors;
    std::vector<Point> particleVelocities;
    const int defaultParticleCount = 50;
    int particleRadius = 3;
};