#include "view.h"
#include <functional>
#include <iostream>
#include <memory>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
SDL_Window *window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

void View::RenderUI()
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

    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return;
    }
    // SDL_RendererInfo info;
    // SDL_GetRendererInfo(renderer, &info);
    // SDL_Log("Current SDL_Renderer: %s", info.name);

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

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

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
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
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

            // ImGui Menu ----------------------------------------------------------------------------------------------------------------------------------------------
            ImGui::Begin("Menu");
            ImGui::SetWindowPos(ImVec2(0, 0));
            ImGui::SetWindowSize(ImVec2(0.2 * SCREEN_WIDTH, ImGui::GetIO().DisplaySize.y));
            CommonShapeSubMenu();
            // ImGui Menu -----------------------------------------------------------------------------------------------------------------------------------------------

            // ImGui Rendering ------------------------------------------------------------------------------------------------------------------------------------------
            ImGui::Render();
            SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);

            ImVec4 clear_color = ImVec4(30.0 / 255.0, 30.0 / 255.0, 30.0 / 255.0, 30.0/255.0);
            SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
            SDL_RenderClear(renderer);

            std::vector<Point> points = {{100, 100}, {200, 200}, {300, 100}, {400, 200}, {150, 250}};
            PointCloudShape_Cvx pointCloudShape_Cvx(points);
            ResolveShapeDefinition(pointCloudShape_Cvx);

            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
            SDL_RenderPresent(renderer);
            // ImGui Rendering ------------------------------------------------------------------------------------------------------------------------------------------
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
}

void View::CommonShapeSubMenu()
{

    if (ImGui::CollapsingHeader("Add Common Shapes", ImGuiTreeNodeFlags_DefaultOpen))
    {
        CircleButton();
    }

    ImGui::End();
}

void View::CircleButton()
{
    char textBuffer[256] = "";
    ImGui::Text("Circle");

    ImGui::InputText("##TextEntry", textBuffer, sizeof(textBuffer));
    ImGui::SameLine();

    if (ImGui::Button("Add"))
    {
    }
}

void View::RenderPointCloudShape_Cvx(SDL_Point *sdlPoints, int pointCount)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawLines(renderer, sdlPoints, pointCount);
    delete[] sdlPoints;
}

void View::CreatePointCloudShape_Cvx(std::vector<Point> points)
{
    SDL_Point *sdlPoints = new SDL_Point[points.size()];

    for (size_t i = 0; i < points.size(); i++)
    {
        sdlPoints[i].x = points[i].x;
        sdlPoints[i].y = points[i].y;
    }
    RenderPointCloudShape_Cvx(sdlPoints, points.size());
}

void View::ResolveShapeDefinition(const Shape &shape)
{
    int shapeID = shape.getID();
    if (shapeID == POINT_CLOUD_SHAPE_CVX)
    {
        const PointCloudShape_Cvx &pointCloudShape_Cvx = dynamic_cast<const PointCloudShape_Cvx &>(shape);
        CreatePointCloudShape_Cvx(pointCloudShape_Cvx.getPoints());
    }
    else
    {
        std::cerr << "SHAPE TYPE IS INVALID (FUNCTION View::ResolveShapeDefinition(const Shape &shape))" << std::endl;
    }
}
