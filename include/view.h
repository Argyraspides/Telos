#pragma once
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "shape.h"

#include <stdio.h>
#include <vector>
#include <SDL.h>

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif
// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../lib/imgui/examples/libs/emscripten/emscripten_mainloop_stub.h"
#endif

class View
{

public:
    void InitSDL();
    void SDLRender();
    void CleanupSDL();

    void InitImGui();
    void CleanupImGui();

    void RenderUI();
    void RenderShape();
    void HUD();

    void CommonShapeSubMenu();
    void CircleButton();

    void ResolveShapeDefinition(const Shape &shape);

    void CreatePointCloudShape_Cvx(std::vector<Point> points);
    void RenderPointCloudShape_Cvx(SDL_Point* sdlPoints, int pointCount);

};