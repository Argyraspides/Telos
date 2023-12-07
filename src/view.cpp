#include "view.h"
#include "model.h"
#include <functional>
#include <iostream>
#include <memory>
// **************************************************************************************************************************************************************************
// MAIN RENDER FUNCTION

void View::Render(Controller *controller, Model *model)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    ImVec4 clear_color = ImVec4(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f);
    // Main loop
    bool done = false;
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!done)
#endif
    {

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // RENDER GUI HERE ***************RENDER GUI HERE*******************RENDER GUI HERE**********************RENDER GUI HERE******************RENDER GUI HERE*************************************************************
        RenderGUI(controller);
        // RENDER GUI HERE ***************RENDER GUI HERE*******************RENDER GUI HERE**********************RENDER GUI HERE******************RENDER GUI HERE*************************************************************

        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);

        // RENDER OBJECTS HERE ***************RENDER OBJECTS HERE*******************RENDER OBJECTS HERE**********************RENDER OBJECTS HERE******************RENDER OBJECTS HERE*************************************************************
        RenderModel(model, controller, renderer);
        // RENDER OBJECTS HERE ***************RENDER OBJECTS HERE*******************RENDER OBJECTS HERE**********************RENDER OBJECTS HERE******************RENDER OBJECTS HERE*************************************************************

        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif
    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}



// **************************************************************************************************************************************************************************
// UI ELEMENTS

void View::UI_Interactive_CommonShapeSubMenu(Controller* controller)
{

    if (ImGui::CollapsingHeader("Add Common Shapes", ImGuiTreeNodeFlags_DefaultOpen))
    {
        UI_Interactive_AddCircleButton(controller);
    }

    ImGui::End();
}

void View::UI_Interactive_AddCircleButton(Controller* controller)
{
    ImGui::Text("Circle");
    static float radius = 10;
    ImGui::InputFloat("Radius", &radius);

    if (ImGui::Button("Add"))
    {
        controller->addPointCloudShape(
            PointCloudShape_Cvx::generateCircle(radius),
            {SCREEN_WIDTH / 2.0F, SCREEN_HEIGHT / 2.0F});
    }
}

void View::UI_ConstructUI(Controller* controller)
{
    ImGui::Begin("Menu");
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(0.2 * SCREEN_WIDTH, ImGui::GetIO().DisplaySize.y));
    UI_Interactive_CommonShapeSubMenu(controller);
}


// **************************************************************************************************************************************************************************
// RENDERING 

void View::RenderPointCloudShape(SDL_Renderer *renderer, std::vector<Point> points)
{
    // TODO: FIND MORE EFFICIENT WAY TO RENDER SHAPES
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int i = 0; i < points.size(); i++)
    {
        SDL_RenderDrawLine(
            renderer,
            points[i].x,
            points[i].y,
            points[(i + 1) % points.size()].x,
            points[(i + 1) % points.size()].y);
    }
}

void View::RenderModel(Model *model, Controller *controller, SDL_Renderer *renderer)
{
    if (model != nullptr)
    {
        for (int i = 0; i < model->getShapeCount(); i++)
        {
            model->getShapeList()[i];
            RenderPointCloudShape(
                renderer,
                ShapeUtils::convertToPointCloud(model->getShapeList()[i]));
        }
    }
}

void View::RenderGUI(Controller *controller)
{
    UI_ConstructUI(controller);
}


// **************************************************************************************************************************************************************************
