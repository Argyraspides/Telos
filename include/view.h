#pragma once

#include "imgui.h"
#include "controller.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "shape.h"
#include "point_cloud_convex.h"
#include "model.h"
#include "application_params.h"
#include <vector>
#include <array>
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

private:
    void Render_Model(SDL_Renderer *renderer);                                                                         // RENDERS THE CONTENTS OF THE PHYSICS ENGINE
    void Render_GUI();                                                                                                 // RENDERS IMGUI COMPONENTS
    void Render_Polygon(SDL_Renderer *renderer, const std::vector<Point> &points, Uint8 r, Uint8 g, Uint8 b, Uint8 a); // RENDERS ANY POLYGON

    // CLEANUP METHODS
private:
    void CleanupSDL(SDL_Renderer *renderer, SDL_Window *window); // CLEANS UP SDL2 UPON APPLICATION EXIT
    void CleanupImGui();                                         // CLEANS UP IMGUI UPON APPLICATION EXIT

private:
    ImGuiIO &SetupImGui(); // SETS UP IMGUI

    //*************************************************** RIDID BODY MECHANICS UI/RENDERING ***************************************************
    // ****************************************************************************************************************************************

private:
    int UI_FetchID();                         // GETS NEXT AVAILABLE ID TO ASSIGN TO AN IMGUI INPUT (TEXTBOX, FLOAT SLIDERS, ETC)
    void UI_Interactive_CommonShapeSubMenu(); // SUBMENU FOR ADDING COMMON SHAPES
    void UI_Interactive_AddRegularPolygonButton();
    void UI_Interactive_AddRectangleButton();
    void UI_Interactive_AddTriangleButton();
    void UI_Interactive_AddArbPolygonInput();

    void UI_Tutorial();              // TUTORIAL AND WELCOME WINDOW
    void UI_About();
    void UI_ModelInfo();             // ENGINE PARAMETERS
    void UI_ShapeInfo();             // INFORMATION ABOUT CURRENT SHAPES ADDED TO THE SCENE
    void UI_CornerInfo(ImGuiIO &io); // FPS AND PAUSE INDICATOR
    bool UI_ModelModError(const MODEL_MODIFICATION_RESULT &result, std::string &errorText, ImVec4& colorText);

    bool m_menuOpen = false;                  // Is the menu currently collapsed or open? (This is to make sure inputs are not handled by the engine behind the menu)
    float m_menuWidth = SCREEN_WIDTH * 0.275; // Width of the menu
    void UI_ConstructMenuModule();            // CREATES THE ENTIRE MENU

private:
    void SDL_EventHandlingLoop();               // HANDLES INPUTS FOR THE VIEWPORT (NOT IMGUI)
    void SDL_ViewportHandler(SDL_Event &event); // HANDLES INPUT ON THE VIEWPORT (I.E. THE AREA THINGS ARE RENDERED)
    void SDL_DragShape(SDL_Event &event);       // ALLOWS USER TO DRAG SHAPES AROUND THE VIEWPORT
    void SDL_RemoveShape(SDL_Event &event);     // ALLOWS USER TO REMOVE SHAPES FROM THE WORLD
    void SDL_Pause(SDL_Event &event);           // ALLOWS USER TO PAUSE THE MODEL
    void SDL_TickModel(SDL_Event &event);       // ALLOWS USER TO MOVE THE MODEL FORWARD OR BACKWARD BY ONE TIME STEP

    static ImVec4 m_currentShapeColor; // Currently selected color of the next shape to be added
    ImVec4 m_clearColor;               // Currently selected color of the background
    Sint16 *m_PCSPointsX;              // Points of a polygon represented as a point cloud in a poniter for rendering with SDL_gfx
    Sint16 *m_PCSPointsY;
    std::vector<std::array<Uint8, 4>> m_PCSColors; // Colors of the polygons that have been added

private:
    Controller *m_controller; // CONTROLLER INTERFACE TO MANIPULATE AND/OR RETRIEVE DATA FROM THE MODEL

    // SDL BOILERPLATE TO SET UP THE WINDOW AND RENDERER
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

    bool done = false;          // Is the entire application done?
    bool m_inputDone = false;   // Are the SDL viewport related control handlers done?
    bool m_renderDone = false;  // Is the ImGui and model rendering portion done?
    bool m_modelPaused = false; // Is the model currently paused?
    static int ImGuiID;         // Next available ID of an ImGui element
    double buttonSpamLimit = 0.5;
};