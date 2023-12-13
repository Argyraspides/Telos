#include "view.h"
#include "model.h"
#include "BUILD_EMCC.h"
#include <functional>
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

View::View(Controller *controller)
{
    this->m_controller = controller;
    this->Render();
}

// **************************************************************************************************************************************************************************
// MAIN RENDER FUNCTION

void View::Render()
{
    // Setup SDL

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return;
    }

    // SDL BOILERPLATE
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("Telos", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);
   // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with SDL_Renderer graphics context

    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return;
    }

    // Setup Dear ImGui context
    ImGuiIO &io = SetupImGui();
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    // Color is #1e1e1e
    ImVec4 clear_color = ImVec4(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f);
    // Main loop
    bool done = false;

    const std::chrono::milliseconds frameDuration(1000 / VIEW_POLLING_RATE);
    auto startTime = std::chrono::high_resolution_clock::now();

#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!done)
#endif
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        if (elapsed >= frameDuration)
        {
            // Handle events such as keyboard/mouse inputs, resizing the window, etc
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT)
                {
                    done = true;
                    this->m_controller->ShutModel();
                }
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                {
                    done = true;
                    this->m_controller->ShutModel();
                }
                SDL_ViewportHandler(event);
            }

            // Start the Dear ImGui frame
            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            // RENDER GUI HERE ***************

            Render_GUI();

            // RENDER GUI HERE ***************

            // Rendering
            ImGui::Render();
            SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
            SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
            SDL_RenderClear(renderer);

            // RENDER OBJECTS HERE ***************

            Render_Model(renderer);

            // RENDER OBJECTS HERE ***************

            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
            SDL_RenderPresent(renderer);
            startTime = std::chrono::high_resolution_clock::now();
        }
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif
    CleanupSDL(renderer, window);
    CleanupImGui();
}

// **************************************************************************************************************************************************************************
// CLEANUP

void View::CleanupSDL(SDL_Renderer *renderer, SDL_Window *window)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void View::CleanupImGui()
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

ImGuiIO &View::SetupImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    return io;
}

// **************************************************************************************************************************************************************************
// UI ELEMENTS

void View::UI_Interactive_CommonShapeSubMenu()
{

    if (ImGui::CollapsingHeader("Add Common Shapes", ImGuiTreeNodeFlags_DefaultOpen))
    {
        UI_Interactive_AddCircleButton();
    }

    ImGui::End();
}

void View::UI_Interactive_AddCircleButton()
{
    ImGui::Text("Circle");
    static float radius = 50;
    static float xVel = 0.0f;
    static float yVel = 0.0f;
    ImGui::InputFloat("Radius", &radius);
    ImGui::InputFloat("X Velocity", &xVel);
    ImGui::InputFloat("Y Velocity", &yVel);

    if (ImGui::Button("Add"))
    {
        PointCloudShape_Cvx circle(PointCloudShape_Cvx::generateCircle(radius));
        circle.m_vel = {xVel, yVel};
        std::shared_ptr<Shape> circleGeneric = std::make_shared<PointCloudShape_Cvx>(circle);
        this->m_controller->UpdateModel_AddShape(circleGeneric, {SCREEN_WIDTH / 2.0F, SCREEN_HEIGHT / 2.0F});
    }
}

void View::UI_ConstructMenuModule()
{
    ImGui::Begin("Menu");
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(300, ImGui::GetIO().DisplaySize.y));
    UI_Interactive_CommonShapeSubMenu();
}

void View::UI_Update()
{
}

// **************************************************************************************************************************************************************************
// RENDERING

void View::Render_PointCloudShape(SDL_Renderer *renderer, std::vector<Point> points)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (size_t i = 0; i < points.size() - 1; ++i)
    {
        SDL_RenderDrawLine(renderer, static_cast<int>(points[i].x), static_cast<int>(points[i].y),
                           static_cast<int>(points[i + 1].x), static_cast<int>(points[i + 1].y));
    }

    SDL_RenderDrawLine(renderer, static_cast<int>(points.back().x), static_cast<int>(points.back().y),
                       static_cast<int>(points.front().x), static_cast<int>(points.front().y));
}

void View::Render_Model(SDL_Renderer *renderer)
{
    if (this->m_controller != nullptr)
    {
        const int shapeCount = this->m_controller->RetrieveModel_GetShapeCount();
        const std::vector<std::shared_ptr<Shape>> &shapeList = this->m_controller->RetrieveModel_ReadShapes();

        for (size_t i = 0; i < shapeCount; i++)
        {
            Render_PointCloudShape(
                renderer,
                ShapeUtils::convertToPointCloud(shapeList[i]));
        }
    }
}

void View::Render_GUI()
{
    UI_ConstructMenuModule();
}

// **************************************************************************************************************************************************************************
// INPUT HANDLING

void View::SDL_ViewportHandler(SDL_Event &event)
{
    SDL_DragShape(event);
    SDL_RemoveShape(event);
    SDL_Pause(event);
}

void View::SDL_DragShape(SDL_Event &event)
{
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
    {
        const std::vector<std::shared_ptr<Shape>> &shapePtrs = this->m_controller->RetrieveModel_ReadShapes();
        for (std::shared_ptr<Shape> shapePtr : shapePtrs)
        {

            if (ShapeUtils::isInside({(float)mouseX, (float)mouseY}, shapePtr))
            {
                // TODO: TEMPORARY, FIND A WAY TO GET EMSCRIPTEN TO ACTUALLY KNOW WHEN THE MOUSE IS RELEASED
#if !BUILD_EMCC
                while (true)
#endif
                {
                    SDL_PollEvent(&event);
                    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
                        break;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    shapePtr->setShapePos(Point({(float)mouseX, (float)mouseY, 0}));
                }
            }
        }
    }
}

void View::SDL_RemoveShape(SDL_Event &event)
{
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT)
    {
        const std::vector<std::shared_ptr<Shape>> &shapePtrs = this->m_controller->RetrieveModel_ReadShapes();
        for (std::shared_ptr<Shape> shapePtr : shapePtrs)
        {
            if (ShapeUtils::isInside({(float)mouseX, (float)mouseY}, shapePtr))
            {
                this->m_controller->UpdateModel_RemoveShape(shapePtr);
                break;
            }
        }
    }
}

void View::SDL_Pause(SDL_Event &event)
{
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
    {
        this->m_controller->PauseUnpauseModel();
    }
}