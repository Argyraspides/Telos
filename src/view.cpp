#include "view.h"
#include "telos_imgui_colors.h"
#include "SDL2_gfxPrimitives.h"
#include "shape_utils.h"
#include "model.h"
#include "BUILD_EMCC.h"
#include <functional>
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <filesystem>

#if BUILD_EMCC
#include <emscripten/emscripten.h>
#endif

int View::ImGuiID = 1;
ImVec4 View::currentShapeColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

View::View(Controller *controller)
{
    this->m_controller = controller;
    this->m_PCSPointsX = new Sint16[m_controller->RetrieveModel_GetMaxPCSPoints()];
    this->m_PCSPointsY = new Sint16[m_controller->RetrieveModel_GetMaxPCSPoints()];
}

View::~View()
{
    delete[] m_PCSPointsX;
    delete[] m_PCSPointsY;
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

    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 separatorColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_Separator] = separatorColor;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    // Color is #1e1e1e

    pthread_t inputThreadId;
    pthread_create(&inputThreadId, nullptr, &View::threadEntry, this);

    const std::chrono::milliseconds frameDuration(1000 / VIEW_POLLING_RATE);
    auto startTime = std::chrono::high_resolution_clock::now();

    clearColor = TELOS_IMGUI_DARKGRAY;

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
                GetFrameEvents().push_back(event);
            }
            renderDone = false;
            for (SDL_Event &_event : GetFrameEvents())
            {
                ImGui_ImplSDL2_ProcessEvent(&_event);
                if (_event.type == SDL_QUIT)
                {
                    done = true;
                    this->m_controller->ShutModel();
                }
                if (_event.type == SDL_WINDOWEVENT && _event.window.event == SDL_WINDOWEVENT_CLOSE && _event.window.windowID == SDL_GetWindowID(window))
                {
                    done = true;
                    this->m_controller->ShutModel();
                }
                if (inputDone)
                    GetFrameEvents().clear();
            }
            renderDone = true;

            // Start the Dear ImGui frame
            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            // RENDER GUI HERE ***************

            Render_GUI();
            UI_FPS(io);

            // RENDER GUI HERE ***************

            // Rendering
            ImGui::Render();
            SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
            SDL_SetRenderDrawColor(renderer, (Uint8)(clearColor.x * 255), (Uint8)(clearColor.y * 255), (Uint8)(clearColor.z * 255), (Uint8)(clearColor.w * 255));
            SDL_RenderClear(renderer);

            // RENDER OBJECTS HERE ***************

            Render_Model(renderer);

            // RENDER OBJECTS HERE ***************

            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
            SDL_RenderPresent(renderer);
            this->ImGuiID = 1;
            startTime = std::chrono::high_resolution_clock::now();
        }
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif
    pthread_join(inputThreadId, nullptr);
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
    io.FontGlobalScale = 0.75f;

    std::string fontPathS;

#if !BUILD_EMCC
    fontPathS = std::filesystem::current_path().string() + "/assets/Roboto/Roboto-Black.ttf";
#else
    fontPathS = "assets/Roboto/Roboto-Black.ttf";
#endif

    io.Fonts->AddFontFromFileTTF(fontPathS.c_str(), 25.0f);

    return io;
}

// **************************************************************************************************************************************************************************
// UI ELEMENTS

int View::UI_FetchID()
{
    return this->ImGuiID++;
}

void View::UI_Interactive_CommonShapeSubMenu()
{

    if (ImGui::CollapsingHeader("Add Common Shapes", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::ColorEdit3("Shape Color", (float *)&currentShapeColor);

        ImGui::NewLine();
        ImGui::NewLine();

        UI_Interactive_AddRegularPolygonButton();

        ImGui::NewLine();

        UI_Interactive_AddRectangleButton();

        ImGui::NewLine();

        UI_Interactive_AddArbPolygonInput();

        ImGui::NewLine();
    }
}

void View::UI_Interactive_AddRegularPolygonButton()
{
    ImGui::Text("Regular Polygon");
    static float radius = 50;
    static float sides = 5;
    static float xVel = 750.0f;
    static float yVel = 250.0f;
    static float rot = 0.0f;
    static float mass = 1.0f;

    ImGui::InputFloat(("Radius##ID" + std::to_string(UI_FetchID())).c_str(), &radius);
    ImGui::InputFloat(("Sides##ID" + std::to_string(UI_FetchID())).c_str(), &sides);
    ImGui::InputFloat(("X Velocity##ID" + std::to_string(UI_FetchID())).c_str(), &xVel);
    ImGui::InputFloat(("Y Velocity##ID" + std::to_string(UI_FetchID())).c_str(), &yVel);
    ImGui::InputFloat(("Rotation##ID" + std::to_string(UI_FetchID())).c_str(), &rot);
    ImGui::InputFloat(("Mass##ID" + std::to_string(UI_FetchID())).c_str(), &mass);

    if (ImGui::Button("Add RP"))
    {
        UI_HandleMaxInputs(xVel, yVel, rot);
        if (sides > m_controller->RetrieveModel_GetMaxPCSPoints())
            sides = m_controller->RetrieveModel_GetMaxPCSPoints();

        PointCloudShape_Cvx regularPoly(Utils::generateRegularPolygon(radius, sides), this->m_controller->RetrieveModel_GetCurrentTime());
        // Engine polls at 30-60 times per second. Input values should be intuitive to the user and hence
        // on the order of once per second, so we divide the values by the engines polling rate.
        // E.g. instead of x velocity being 8 pixels every 20ms, its 8 pixels every second.

        // It shouldn't really be the views job to determine what is a maximum input -- that should be decided for the engine.
        // Think of a better solution in future.

        Uint8 r = (Uint8)(currentShapeColor.x * pixelLimit);
        Uint8 g = (Uint8)(currentShapeColor.y * pixelLimit);
        Uint8 b = (Uint8)(currentShapeColor.z * pixelLimit);
        Uint8 a = (Uint8)(currentShapeColor.w * pixelLimit);

        std::array<Uint8, 4> color = {r, g, b, a};
        m_PCSColors.push_back(color);

        regularPoly.m_vel = {xVel / ENGINE_POLLING_RATE, yVel / ENGINE_POLLING_RATE};
        regularPoly.m_rot = rot / ENGINE_POLLING_RATE;
        regularPoly.m_mass = mass;
        std::shared_ptr<Shape> polyGeneric = std::make_shared<PointCloudShape_Cvx>(regularPoly);
        this->m_controller->UpdateModel_AddShape(polyGeneric, {SCREEN_WIDTH / 2.0F, SCREEN_HEIGHT / 2.0F});
    }
}

void View::UI_Interactive_AddRectangleButton()
{
    ImGui::Text("Rectangle");
    static float w = 250, h = 50;
    static float xVel = 1000.0f;
    static float yVel = 2500.0f;
    static float rot = 0.5f;
    static float mass = 1.0f;
    ImGui::InputFloat(("Width##ID" + std::to_string(UI_FetchID())).c_str(), &w);
    ImGui::InputFloat(("Height##ID" + std::to_string(UI_FetchID())).c_str(), &h);
    ImGui::InputFloat(("X Velocity##ID" + std::to_string(UI_FetchID())).c_str(), &xVel);
    ImGui::InputFloat(("Y Velocity##ID" + std::to_string(UI_FetchID())).c_str(), &yVel);
    ImGui::InputFloat(("Rotation##ID" + std::to_string(UI_FetchID())).c_str(), &rot);
    ImGui::InputFloat(("Mass##ID" + std::to_string(UI_FetchID())).c_str(), &mass);

    if (ImGui::Button("Add Rect"))
    {
        UI_HandleMaxInputs(xVel, yVel, rot);

        Uint8 r = (Uint8)(currentShapeColor.x * pixelLimit);
        Uint8 g = (Uint8)(currentShapeColor.y * pixelLimit);
        Uint8 b = (Uint8)(currentShapeColor.z * pixelLimit);
        Uint8 a = (Uint8)(currentShapeColor.w * pixelLimit);

        std::array<Uint8, 4> color = {r, g, b, a};
        m_PCSColors.push_back(color);

        PointCloudShape_Cvx Rectangle(Utils::generateRectangle(w, h), this->m_controller->RetrieveModel_GetCurrentTime());
        Rectangle.m_vel = {xVel / ENGINE_POLLING_RATE, yVel / ENGINE_POLLING_RATE};
        Rectangle.m_rot = rot / ENGINE_POLLING_RATE;
        Rectangle.m_mass = mass;

        std::shared_ptr<Shape> RectangleGeneric = std::make_shared<PointCloudShape_Cvx>(Rectangle);
        this->m_controller->UpdateModel_AddShape(RectangleGeneric, {SCREEN_WIDTH / 2.0F, SCREEN_HEIGHT / 2.0F});
    }
}

void View::UI_Interactive_AddArbPolygonInput()
{
    ImGui::Text("Arbitrary Shape");
    static char inputBuf[256] = "(0,0),(200,100),(400,300),(500,500),(300,700),(100,600)";
    static float xVel = 69.0f;
    static float yVel = 69.0f;
    static float rot = 1.0f;
    static float mass = 1.0f;
    static ImVec4 invalidInputTxtColor = TELOS_IMGUI_CLEAR;

    ImGui::InputText(("Points##ID" + std::to_string(UI_FetchID())).c_str(), inputBuf, sizeof(inputBuf));
    ImGui::InputFloat(("X Velocity##ID" + std::to_string(UI_FetchID())).c_str(), &xVel);
    ImGui::InputFloat(("Y Velocity##ID" + std::to_string(UI_FetchID())).c_str(), &yVel);
    ImGui::InputFloat(("Rotation##ID" + std::to_string(UI_FetchID())).c_str(), &rot);
    ImGui::InputFloat(("Mass##ID" + std::to_string(UI_FetchID())).c_str(), &mass);

    if (ImGui::Button("Add AS"))
    {
        std::vector<Point> pts = Utils::generateArbPoly2D(std::string(inputBuf));
        if (pts.size() > 1 && pts.size() <= m_controller->RetrieveModel_GetMaxPCSPoints())
        {
            UI_HandleMaxInputs(xVel, yVel, rot);

            Uint8 r = (Uint8)(currentShapeColor.x * pixelLimit);
            Uint8 g = (Uint8)(currentShapeColor.y * pixelLimit);
            Uint8 b = (Uint8)(currentShapeColor.z * pixelLimit);
            Uint8 a = (Uint8)(currentShapeColor.w * pixelLimit);

            std::array<Uint8, 4> color = {r, g, b, a};
            m_PCSColors.push_back(color);

            PointCloudShape_Cvx arbPoly(pts, this->m_controller->RetrieveModel_GetCurrentTime());
            arbPoly.m_vel = {xVel / ENGINE_POLLING_RATE, yVel / ENGINE_POLLING_RATE};
            arbPoly.m_rot = rot / ENGINE_POLLING_RATE;
            arbPoly.m_mass = mass;

            std::shared_ptr<Shape> arbPolyGeneric = std::make_shared<PointCloudShape_Cvx>(arbPoly);
            this->m_controller->UpdateModel_AddShape(arbPolyGeneric, {SCREEN_WIDTH / 2.0F, SCREEN_HEIGHT / 2.0F});

            invalidInputTxtColor = TELOS_IMGUI_CLEAR;
            DBL_MAX;
        }
        else
        {
            invalidInputTxtColor = TELOS_IMGUI_RED;
        }
    }

    ImGui::TextColored(invalidInputTxtColor, "Invalid input!");
}

void View::UI_ConstructMenuModule()
{
    static float windowWidth = SCREEN_WIDTH * 0.275;
    ImGui::Begin("Menu");
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(windowWidth, ImGui::GetIO().DisplaySize.y));

    UI_ModelInfo();
    UI_Interactive_CommonShapeSubMenu();
    UI_ShapeInfo();

    ImGui::End();
}

void View::UI_HandleMaxInputs(float &xVel, float &yVel, float &rot)
{
    if (xVel > m_controller->RetrieveModel_GetMaxVelocity().x)
        xVel = m_controller->RetrieveModel_GetMaxVelocity().x;
    if (yVel > m_controller->RetrieveModel_GetMaxVelocity().y)
        yVel = m_controller->RetrieveModel_GetMaxVelocity().y;
    if (rot > m_controller->RetrieveModel_GetMaxRotVelocity())
        rot = m_controller->RetrieveModel_GetMaxRotVelocity();
}

void View::UI_ModelInfo()
{
    static float e = 1.0f;
    static float wallE = 1.0f; // WWWWAAAAAALLLLLL-E
    if (ImGui::CollapsingHeader("Engine Parameters", ImGuiTreeNodeFlags_CollapsingHeader))
    {
        ImGui::TextColored(TELOS_IMGUI_WHITE, "Engine time step: %.3fs", m_controller->RetrieveModel_GetTimeStep());
        ImGui::TextColored(TELOS_IMGUI_WHITE, "Time elapsed: %.3fs", m_controller->RetrieveModel_GetCurrentTime());

        ImGui::NewLine();

        ImGui::TextColored(TELOS_IMGUI_RED0, "Maximum allowed velocities: (%.3f, %.3f) px/s", m_controller->RetrieveModel_GetMaxVelocity().x, m_controller->RetrieveModel_GetMaxVelocity().y);
        ImGui::TextColored(TELOS_IMGUI_RED0, "Maximum allowed rotational velocity: %.3f rad/s", m_controller->RetrieveModel_GetMaxRotVelocity());
        ImGui::TextColored(TELOS_IMGUI_RED0, "Maximum allowed shapes: %i", m_controller->RetrieveModel_GetMaxObjects());
        ImGui::TextColored(TELOS_IMGUI_RED0, "Maximum energy conservation violation: %.10f Joules", m_controller->RetrieveModel_GetMaxEnergyViolation());
        ImGui::TextColored(TELOS_IMGUI_RED0, "Maximum shape vertices: %i", m_controller->RetrieveModel_GetMaxPCSPoints());
        ImGui::NewLine();

        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5); // Set the next added elements to this width

        ImGui::ColorEdit3("Background Color", (float *)&clearColor);
        ImGui::SliderFloat("Collision Elasticity", &e, m_controller->RetrieveModel_GetMinElasticity(), m_controller->RetrieveModel_GetMaxElasticity());
        ImGui::SliderFloat("Wall Collision Elasticity", &wallE, m_controller->RetrieveModel_GetMinElasticity(), m_controller->RetrieveModel_GetMaxElasticity());
       
        ImGui::PopItemWidth(); // Restore default item width

        m_controller->UpdateModel_ChangeElasticity(e);
        m_controller->UpdateModel_ChangeWallElasticity(wallE);

        if(ImGui::Button("Delete All Shapes"))
        {
            m_controller->UpdateModel_RemoveAllShapes();
        }
    }
}

void View::UI_FPS(ImGuiIO &io)
{

    static bool showFps = true;

    if (showFps)
    {
        ImGui::SetNextWindowPos(ImVec2(SCREEN_WIDTH - 90, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(90, 50), ImGuiCond_Always);
        ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
        ImVec2 textSize = ImGui::CalcTextSize("999.9 fps");
        ImVec2 textPosition = ImVec2((ImGui::GetWindowWidth() - textSize.x) * 0.5f, (ImGui::GetWindowHeight() - textSize.y) * 0.5f);
        ImGui::SetCursorPos(textPosition);
        ImGui::Text("%.1f FPS", io.Framerate);
        ImGui::End();
    }
}

void View::UI_Tutorial()
{
    static bool showTutorial = true;
    static Point center = {SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0};

    if (showTutorial)
    {
        static const char *tutText =
            "Welcome to Telos! A 2D rigidbody physics engine\n\n"

            "While still incomplete, with many features to come,\n"
            "the engine is functional and you may play around with\n"
            "it as development progresses.\n\n"

            "Controls:\n\n"

            "Esc: Pause\n"
            "Left click hold: Drag shapes around\n"
            "Right click: Delete a shape\n"
            "Left and right arrows: move backward/forward by one time step (you must pause first)\n";

        ImGui::SetNextWindowPos(ImVec2(center.x, center.y), ImGuiCond_FirstUseEver);
        ImGui::Begin("Tutorial", &showTutorial);
        ImGui::Text("%s", tutText);

        if (ImGui::Button("Close"))
            showTutorial = false;
        ImGui::End();
    }
}

void View::UI_ShapeInfo()
{
    if (ImGui::CollapsingHeader("Shape Info", ImGuiTreeNodeFlags_DefaultOpen))
    {
        std::vector<std::shared_ptr<Shape>> shapeList = this->m_controller->RetrieveModel_ReadShapes();
        for (const auto &shape : shapeList)
        {
            const Point &center = shape->m_center;
            const Point &vel = shape->m_vel;
            const double rot = shape->m_rot;
            const double mass = shape->m_mass;
            const long long id = shape->m_shapeID;
            const double rotInert = shape->m_rotInert;
            const double ek = Utils::getTranslationalKineticEnergy(*shape);
            const double ekrot = Utils::getRotationalKineticEnergy(*shape);
            const double timeSpawned = shape->m_timeSpawned;
            std::string shapeName = shape->m_name;

            ImGui::TextColored(TELOS_IMGUI_RED, "Shape #%lld (%s)", id, shapeName.c_str());
            ImGui::TextColored(TELOS_IMGUI_WHITE, "Time added: %.3fs", timeSpawned);
            ImGui::TextColored(TELOS_IMGUI_WHITE, "Center: (%f, %f)", center.x, center.y);
            ImGui::TextColored(TELOS_IMGUI_BLUE, "Velocity: (%f, %f) px/s", vel.x * ENGINE_POLLING_RATE, vel.y * ENGINE_POLLING_RATE);
            ImGui::TextColored(TELOS_IMGUI_LIGHTGRAY2, "Rotational Velocity: %f rad/s", rot * ENGINE_POLLING_RATE);
            ImGui::TextColored(TELOS_IMGUI_GREEN, "Mass: %f kg", mass);
            ImGui::TextColored(TELOS_IMGUI_PURPLE, "Rotational Inertia: %f ML^2", rotInert);
            ImGui::TextColored(TELOS_IMGUI_LIGHTBLUE, "Rotational Kinetic Energy: %f J", ekrot);
            ImGui::TextColored(TELOS_IMGUI_LIGHTBLUE, "Translational Kinetic Energy: %f J", ek);
            ImGui::TextColored(TELOS_IMGUI_LIGHTBLUE, "Total Kinetic Energy: %f J", ek + ekrot);
            ImGui::NewLine();
            ImGui::Separator();
            ImGui::NewLine();
        }
    }
}

// **************************************************************************************************************************************************************************
// RENDERING

void View::Render_PointCloudShape(SDL_Renderer *renderer, std::vector<Point> points, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{

    for (int i = 0; i < points.size(); i++)
    {
        m_PCSPointsX[i] = points[i].x;
        m_PCSPointsY[i] = points[i].y;
    }

    filledPolygonRGBA(renderer, m_PCSPointsX, m_PCSPointsY, points.size(), r, g, b, a);
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
                Utils::convertToPointCloud(shapeList[i]), m_PCSColors[i][0], m_PCSColors[i][1], m_PCSColors[i][2], m_PCSColors[i][3]);
        }
    }
}

void View::Render_GUI()
{
    UI_ConstructMenuModule();
    UI_Tutorial();
}

// **************************************************************************************************************************************************************************
// INPUT HANDLING

void View::SDL_EventHandlingLoop()
{
    const std::chrono::milliseconds frameDuration(1000 / VIEW_INPUT_POLLING_RATE);
    auto startTime = std::chrono::high_resolution_clock::now();
    while (!done)
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        inputDone = false;
        if (elapsed >= frameDuration)
        {
            for (SDL_Event &event : GetFrameEvents())
            {
                SDL_ViewportHandler(event);
                if (renderDone)
                    GetFrameEvents().clear();
            }
            inputDone = true;
            startTime = std::chrono::high_resolution_clock::now();
        }
    }
}

void View::SDL_ViewportHandler(SDL_Event &event)
{
    SDL_DragShape(event);
    SDL_RemoveShape(event);
    SDL_Pause(event);
    SDL_TickModel(event);
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

            if (Utils::isInside({(double)mouseX, (double)mouseY}, shapePtr))
            {
                this->m_controller->PauseModel();
                this->clearColor = TELOS_IMGUI_LIGHTGRAY;
                while (true)
                {
                    SDL_PollEvent(&event);
                    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
                    {
                        this->m_controller->UnpauseModel();
                        this->clearColor = TELOS_IMGUI_DARKGRAY;
                        break;
                    }
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
            if (Utils::isInside({(float)mouseX, (float)mouseY}, shapePtr))
            {
                m_PCSColors.erase(m_PCSColors.begin() + shapePtr->getShapeID());
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
        if (!enginePaused)
        {
            this->clearColor = TELOS_IMGUI_LIGHTGRAY;
            this->m_controller->PauseModel();
            enginePaused = true;
        }
        else
        {
            this->clearColor = TELOS_IMGUI_DARKGRAY;
            this->m_controller->UnpauseModel();
            enginePaused = false;
        }
    }
}

void View::SDL_TickModel(SDL_Event &event)
{
    if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d))
    {
        this->m_controller->UpdateModel_ForwardTick();
    }
    else if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a))
    {
        this->m_controller->UpdateModel_BackwardTick();
    }
}