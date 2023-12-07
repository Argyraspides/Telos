#pragma once
#include "imgui.h"
#include "controller.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "shape.h"
#include "model.h"

#include <stdio.h>
#include <vector>
#include <SDL.h>

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif
// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../lib/imgui/examples/libs/emscripten/emscripten_mainloop_stub.h"
#endif

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

class View
{

public:
// RENDER METHODS

    void Render(Controller *controller, Model *model);
    void RenderModel(Model *model, Controller* controller, SDL_Renderer* renderer); 
    void RenderGUI(Controller *controller);
    void RenderPointCloudShape(SDL_Renderer *renderer, std::vector<Point> points);

public:
// UI ELEMENT METHODS
    void UI_Interactive_CommonShapeSubMenu(Controller* controller);
    void UI_Interactive_AddCircleButton(Controller* controller);
    void UI_ConstructUI(Controller* controller);
    void UI_Update(Controller* controller);

};