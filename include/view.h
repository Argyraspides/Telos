#pragma once

#include "imgui.h"
#include "controller.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "shape.h"
#include "model.h"

#include <vector>
#include <SDL.h>

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

#ifdef __EMSCRIPTEN__
#include "../lib/imgui/examples/libs/emscripten/emscripten_mainloop_stub.h"
#endif

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

class View
{

public:
    View(Controller *controller);
    void Render();

private:
    void Render_Model(SDL_Renderer *renderer);
    void Render_GUI();
    void Render_PointCloudShape(SDL_Renderer *renderer, std::vector<Point> points);

    // CLEANUP METHODS
private:
    void CleanupSDL(SDL_Renderer *renderer, SDL_Window *window);
    void CleanupImGui();

private:
    // SETUP METHODS
    ImGuiIO &SetupImGui();

private:
    // UI ELEMENT METHODS
    void UI_Interactive_CommonShapeSubMenu();
    void UI_Interactive_AddCircleButton();
    void UI_ConstructMenuModule();
    void UI_Update();

private:
    // USER INPUT HANDLING METHODS
    void SDL_ViewportHandler(SDL_Event &event);
    void SDL_DragShape(SDL_Event &event);
    void SDL_RemoveShape(SDL_Event &event);


private:
    Controller *m_controller;
};