#pragma once
#include "cartesian.h"
#include "SDL.h"
#include "imgui.h"
#include <vector>
#include <array>

enum ANIMATION_TYPE
{
    PARTICLE_EXPLOSION = 0,
    WALLPAPER = 1
};

namespace AnimationUtils
{
    static SDL_Color generateRandomSDLColor()
    {
        SDL_Color color;
        color.r = rand() % 256; 
        color.g = rand() % 256;
        color.b = rand() % 256; 
        color.a = 255;   
        return color;       
    }

    static SDL_Color imGuiToSDLColor(ImVec4 c)
    {
        SDL_Color color;
        color.r = c.x * 255.0;
        color.g = c.y * 255.0;
        color.b = c.z * 255.0;
        color.a = c.w * 255.0;
        return color;
    }

};

class Animation
{
public:
    Animation(double duration);
    virtual void tick(SDL_Renderer *renderer){};
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

    void tick(SDL_Renderer *renderer) override;

    std::vector<Point> particles;
    std::vector<SDL_Color> particleColors;
    std::vector<Point> particleVelocities;
    const int defaultParticleCount = 10;
    int particleRadius = 3;
};

class BackgroundGradientAnimation : public Animation
{
public:
    BackgroundGradientAnimation(double duration);
    BackgroundGradientAnimation(double duration, SDL_Color startingColor);
    void constructRandomGradient(SDL_Color c);
    std::vector<SDL_Color> bandColors;
    int bandWidth = 10;
    void tick(SDL_Renderer *renderer) override;
};