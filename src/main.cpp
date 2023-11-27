#include "BUILD_EMCC.h"
#include <SDL2/SDL.h>

#if BUILD_EMCC
    #include <emscripten.h>
#endif

#include <cmath>

SDL_Window* pwindow;
SDL_Renderer* prenderer;
bool first_call = true;

void loop()
{
    // Circle parameters
    int circleRadius = 100;
    int circleX = 1280/2;
    int circleY = 720/2;

    if (first_call)
    {
        pwindow = SDL_CreateWindow(
            "boing",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            1280,
            720,
            SDL_WINDOW_RESIZABLE
        );

        prenderer = SDL_CreateRenderer(
            pwindow,
            -1,
            0
        );

        first_call = false;
    }

    SDL_SetRenderDrawColor(prenderer, 50, 50, 50, 255);
    SDL_RenderClear(prenderer);

    SDL_SetRenderDrawColor(prenderer, 255, 255, 255, 255);
    for (int i = 0; i < 360; ++i)
    {
        int x = static_cast<int>(circleX + circleRadius * cos(i * 3.14159265 / 180));
        int y = static_cast<int>(circleY + circleRadius * sin(i * 3.14159265 / 180));
        SDL_RenderDrawPoint(prenderer, x, y);
    }

    SDL_RenderPresent(prenderer);
}

int main(int, char**)
{
    #if BUILD_EMCC
        emscripten_set_main_loop(loop, 60, 0);
    #endif

    #if !BUILD_EMCC
        loop();
    #endif
}
