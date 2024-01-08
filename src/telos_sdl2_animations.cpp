#include "telos_sdl2_animations.h"
#include <random>
#include <chrono>
#include "application_params.h"

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
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distributionVel(-100, 100);
    std::uniform_int_distribution<int> distributionColors(0, 255);

    for (int i = 0; i < defaultParticleCount; i++)
    {
        particles.push_back(startPosition);
        particleVelocities.push_back({(double)distributionVel(gen), (double)distributionVel(gen), 0});

        int r = distributionColors(gen);
        int g = distributionColors(gen);
        int b = distributionColors(gen);
        int a = 1;

        std::array<int, 4> color = {r, g, b, a};

        particleColors.push_back(color);
    }
}

void ParticleExplosionAnimation::tick(SDL_Renderer *renderer)
{
    for (int i = 0; i < particles.size(); i++)
    {
        particles[i] = particles[i] + particleVelocities[i];
        SDL_SetRenderDrawColor(renderer, particleColors[i][0], particleColors[i][1], particleColors[i][2], particleColors[i][3]);
        SDL_Rect particle;
        particle.w = particleRadius;
        particle.h = particleRadius;
        particle.x = particles[i].x;
        particle.y = particles[i].y;
        SDL_RenderFillRect(renderer, &particle);
    }
    timeElapsed += 0.1;
}
