#pragma once

#include "imgui.h"
#include "telos_imgui_colors.h"
#include "controller.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "model.h"

#include <vector>
#include <array>
#include <SDL.h>
#include <pthread.h>

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

// EMSCRIPTEN REQUIRES THE ENTIRE APPLICATION TO BE BUNDLED INSIDE OF A SINGLE FUNCTION LOOP THAT IS HANDED OVER TO EMSCRIPTEN.
// HERE IS A STOWED AWAY "HACK" THAT LETS US DEFINE WHICH PART OF OUR CODE WE WANT EMSCRIPTEN TO WORRY ABOUT WITHOUT NEEDING TO
// GIVE OURSELVES A HEADACHE BY DESIGNING THE APPLICATION AROUND EMSCRIPTENS REQUIREMENTS.
#ifdef __EMSCRIPTEN__
#include "../lib/imgui/examples/libs/emscripten/emscripten_mainloop_stub.h"
#endif

class View
{

public:
    View(Controller *controller);
    ~View();

    // AN ENTRY POINT FOR A THREAD TO THE NEW THREAD THAT WILL BE CREATED TO HANDLE INPUTS ON THE WORLD SCENE, SEPARATE FROM IMGUI INPUTS
    static void *threadEntry(void *instance)
    {
        reinterpret_cast<View *>(instance)->SDL_EventHandlingLoop();
        return nullptr;
    }
    void Render(); // SETS UP SDL2, DEAR IMGUI, AND BEGINS THE RENDER & INPUT LOOPS

    void CleanupSDL(SDL_Renderer *renderer, SDL_Window *window); // CLEANS UP SDL2 UPON APPLICATION EXIT
    void CleanupImGui();                                         // CLEANS UP IMGUI UPON APPLICATION EXIT
    ImGuiIO &SetupImGui();                                       // SETS UP IMGUI

    void SDL_EventHandlingLoop(); // LOOP FOR SDL2 INPUTS (RUNS IN SEPARATE THREAD FROM IMGUI)
    void SDL_ViewportHandler(SDL_Event &event); // ADD FUNCTIONS INTO THIS FUNCTION THAT HANDLES DIFFERENT INPUTS

    Controller *m_controller; // CONTROLLER INTERFACE TO MANIPULATE AND/OR RETRIEVE DATA FROM THE MODEL

    // SDL BOILERPLATE TO SET UP THE WINDOW AND RENDERER
    int SCREEN_WIDTH = 1280, SCREEN_HEIGHT = 720;
    int VIEW_POLLING_RATE = 60;
    int VIEW_INPUT_POLLING_RATE = 60;
    ImVec4 m_clearColor = TELOS_IMGUI_DARKGRAY;
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("Telos", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    // SDL WASN'T REALLY MEANT TO BE MULTITHREADED. AS A WORKAROUND, THE THREAD WHICH CREATED THE WINDOW AND RENDERER WILL CONSTANTLY POLL EVENTS AND ADD IT TO THIS
    // VECTOR ARRAY, WHERE OTHER THREADS CAN INSPECT THEM, RATHER THAN EACH THREAD CREATING ITS OWN EVENTS WHICH GIVES WEIRD BEHAVIOR
    std::vector<SDL_Event> &GetFrameEvents()
    {
        static std::vector<SDL_Event> frame_events;
        return frame_events;
    }

    bool done = false;         // Is the entire application done?
    bool m_inputDone = false;  // Are the SDL viewport related control handlers done?
    bool m_renderDone = false; // Is the ImGui and model rendering portion done?
};