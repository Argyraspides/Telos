#include "telos_sdl2_animations.h"
#include <random>
#include <chrono>

Animation::Animation(double duration)
{
    this->duration = duration;
    this->timeElapsed = 0;
}


ParticleExplosionAnimation::ParticleExplosionAnimation(Coordinate startPosition, double duration) : Animation(duration)
{
    this->duration = duration;
    this->animationType = ANIMATION_TYPE::PARTICLE_EXPLOSION;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distributionVel(-100, 100);

    for (int i = 0; i < defaultParticleCount; i++)
    {
        Pixel pixel(startPosition.x, startPosition.y, AnimationUtils::generateRandomSDLColor());
        Coordinate velocity(distributionVel(gen), distributionVel(gen));
        particles.push_back(pixel);
        particleVelocities.push_back(velocity);
    }
}

void ParticleExplosionAnimation::tick(SDL_Renderer *renderer)
{
    for (int i = 0; i < particles.size(); i++)
    {
        particles[i].x += particleVelocities[i].x;
        particles[i].y += particleVelocities[i].y;

        SDL_SetRenderDrawColor(renderer, particles[i].color.r, particles[i].color.g, particles[i].color.b, particles[i].color.a);

        SDL_Rect particle;
        particle.w = particleRadius;
        particle.h = particleRadius;
        particle.x = particles[i].x;
        particle.y = particles[i].y;

        SDL_RenderFillRect(renderer, &particle);
    }
    timeElapsed += 0.1;
}

BackgroundGradientAnimation::BackgroundGradientAnimation(double duration) : Animation(duration)
{
    this->animationType = ANIMATION_TYPE::WALLPAPER;
    constructRandomGradient(AnimationUtils::generateRandomSDLColor());
}

BackgroundGradientAnimation::BackgroundGradientAnimation(double duration, SDL_Color startingColor, int SCREEN_WIDTH, int SCREEN_HEIGHT) : Animation(duration)
{
    this->SCREEN_WIDTH = SCREEN_WIDTH;
    this->SCREEN_HEIGHT = SCREEN_HEIGHT;

    this->animationType = ANIMATION_TYPE::WALLPAPER;
    constructRandomGradient(startingColor);
}

void BackgroundGradientAnimation::constructRandomGradient(SDL_Color c)
{
    int iterations = SCREEN_WIDTH / bandWidth;
    for(int i = 0; i < iterations; i++)
    {
        if (c.r - 1 > 0) c.r--;
        else if(c.g - 1 > 0) c.g--;
        else if(c.b - 1 > 0) c.b--;
        bandColors.push_back(c);
    }
}

void BackgroundGradientAnimation::tick(SDL_Renderer *renderer)
{
    int iterations = SCREEN_WIDTH / bandWidth;
    for (int i = 0; i < iterations; i++)
    {
        SDL_Color color = bandColors[i];

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect band;
        band.w = bandWidth;
        band.h = SCREEN_HEIGHT;
        band.x = i * bandWidth;
        band.y = 0;
        SDL_RenderFillRect(renderer, &band);
    }
}