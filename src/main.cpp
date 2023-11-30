#include "BUILD_EMCC.h"
#include <SDL.h>
#include <cmath>

#if BUILD_EMCC
    #include <emscripten.h>
#endif

SDL_Window *pwindow;
SDL_Renderer *prenderer;
bool first_call = true;

float circleX = 1280 / 2;
float circleY = 720 / 2;
float xVec = 1;
float yVec = 1;
int circleRadius = 100;

void loop()
{
    // Circle parameters
    circleX+=0.1;
    circleY+=0.1;

    circleY*=yVec;
    circleX*=xVec;
    

    if (first_call)
    {
        pwindow = SDL_CreateWindow(
            "boing",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            1280,
            720,
            SDL_WINDOW_RESIZABLE);

        prenderer = SDL_CreateRenderer(
            pwindow,
            -1,
            0);

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

    if (1280 - circleX <= circleRadius || circleX <= circleRadius)
    {
        xVec*=-1;
    }
    if (720 - circleY <= circleRadius || circleY <= circleRadius)
    {
        yVec*=-1;
    }

    SDL_RenderPresent(prenderer);
}

int main(int, char **)
{
#if BUILD_EMCC
    emscripten_set_main_loop(loop, 120, 0);
#else
    while (true)
        loop();
#endif
}
