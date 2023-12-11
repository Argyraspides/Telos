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

// EMSCRIPTEN REQUIRES THE ENTIRE APPLICATION TO BE BUNDLED INSIDE OF A SINGLE FUNCTION LOOP THAT IS HANDED OVER TO EMSCRIPTEN.
// HERE IS A STOWED AWAY "HACK" THAT LETS US DEFINE WHICH PART OF OUR CODE WE WANT EMSCRIPTEN TO WORRY ABOUT WITHOUT NEEDING TO 
// GIVE OURSELVES A HEADACHE BY DESIGNING THE APPLICATION AROUND EMSCRIPTENS REQUIREMENTS.
#ifdef __EMSCRIPTEN__
#include "../lib/imgui/examples/libs/emscripten/emscripten_mainloop_stub.h"
#endif

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define VIEW_POLLING_RATE 30 // POLLS PER SECOND

class View
{

public:
    View(Controller *controller);

private:
    void Render();                                                                  // SETS UP SDL2, DEAR IMGUI, AND BEGINS THE RENDER & INPUT LOOPS
    void Render_Model(SDL_Renderer *renderer);                                      // RENDERS THE CONTENTS OF THE PHYSICS ENGINE
    void Render_GUI();                                                              // RENDERS IMGUI COMPONENTS
    void Render_PointCloudShape(SDL_Renderer *renderer, std::vector<Point> points); // RENDERS A SHAPE OF TYPE POINT CLOUD

    // CLEANUP METHODS
private:
    void CleanupSDL(SDL_Renderer *renderer, SDL_Window *window); // CLEANS UP SDL2 UPON APPLICATION EXIT
    void CleanupImGui();                                         // CLEANS UP IMGUI UPON APPLICATION EXIT

private:
    ImGuiIO &SetupImGui(); // SETS UP IMGUI

private:
    // THESE ARE ALL FUNCTIONS WHICH DEFINE UI COMPONENTS OF IMGUI, INCLUDING MENUS, BUTTONS, TEXT BOXES, ETC
    void UI_Interactive_CommonShapeSubMenu();
    void UI_Interactive_AddCircleButton();
    void UI_ConstructMenuModule();

    void UI_Update(); // UPDATES ALL THE UI CONTENTS

private:
    void SDL_ViewportHandler(SDL_Event &event); // HANDLES INPUT ON THE VIEWPORT (I.E. THE AREA THINGS ARE RENDERED)
    void SDL_DragShape(SDL_Event &event);       // ALLOWS USER TO DRAG SHAPES AROUND THE VIEWPORT
    void SDL_RemoveShape(SDL_Event &event);     // ALLOWS USER TO REMOVE SHAPES FROM THE WORLD
    void SDL_Pause(SDL_Event &event);           // ALLOWS USER TO PAUSE THE MODEL

private:
    Controller *m_controller; // CONTROLLER INTERFACE TO MANIPULATE AND/OR RETRIEVE DATA FROM THE MODEL

    // SDL BOILERPLATE
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("Telos", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
};