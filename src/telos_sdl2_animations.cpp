#include "telos_sdl2_animations.h"
#include "application_params.h"

#include <random>
#include <chrono>

Animation::Animation(double duration)
{
    this->duration = duration;
    this->timeElapsed = 0;
}

ParticleExplosionAnimation::ParticleExplosionAnimation(Point startPosition, double duration, int particleCount) : Animation(duration)
{
}

ParticleExplosionAnimation::ParticleExplosionAnimation(Point startPosition, double duration) : Animation(duration)
{
    this->duration = duration;
    this->animationType = ANIMATION_TYPE::PARTICLE_EXPLOSION;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distributionVel(-100, 100);

    for (int i = 0; i < defaultParticleCount; i++)
    {
        particles.push_back(startPosition);
        particleVelocities.push_back({(double)distributionVel(gen), (double)distributionVel(gen), 0});

        SDL_Color color = AnimationUtils::generateRandomSDLColor();
        particleColors.push_back(color);
    }
}

void ParticleExplosionAnimation::tick(SDL_Renderer *renderer)
{
    for (int i = 0; i < particles.size(); i++)
    {
        particles[i] = particles[i] + particleVelocities[i];
        SDL_SetRenderDrawColor(renderer, particleColors[i].r, particleColors[i].g, particleColors[i].b, particleColors[i].a);
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

BackgroundGradientAnimation::BackgroundGradientAnimation(double duration, SDL_Color startingColor) : Animation(duration)
{
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
