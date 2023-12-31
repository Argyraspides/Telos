#include "view.h"
#include "SDL2_gfxPrimitives.h"
#include "telos_sdl2_animations.h"
#include "shape_utils.h"
#include "model.h"
#include "BUILD_EMCC.h"
#include "telos_imgui_colors.h"

#if BUILD_EMCC
#include <emscripten/emscripten.h>
#include "emscripten_browser_clipboard.h"
#endif

#include <functional>
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <filesystem>

int View::ImGuiID = 1;
ImVec4 View::m_currentShapeColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

View::View(Controller *controller)
{
    m_controller = controller;
    m_PCSPointsX = new Sint16[m_controller->RetrieveModel_GetMaxPCSPoints()];
    m_PCSPointsY = new Sint16[m_controller->RetrieveModel_GetMaxPCSPoints()];
}

View::~View()
{
    delete[] m_PCSPointsX;
    delete[] m_PCSPointsY;
}

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

    pthread_t inputThreadId;
    pthread_create(&inputThreadId, nullptr, &View::threadEntry, this);

    const std::chrono::milliseconds frameDuration(1000 / VIEW_POLLING_RATE);
    auto startTime = std::chrono::high_resolution_clock::now();

    m_clearColor = TELOS_IMGUI_DARKGRAY;
    m_gradientClearColor = ImVec4(45.0 / 255.0, 144.0 / 255.0, 149.0 / 255.0, 1.0);
    BackgroundGradientAnimation bg(1, AnimationUtils::imGuiToSDLColor(m_gradientClearColor));
    backgroundAnimations.push_back(std::make_shared<BackgroundGradientAnimation>(bg));

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
            m_renderDone = false;
            for (SDL_Event &_event : GetFrameEvents())
            {
                ImGui_ImplSDL2_ProcessEvent(&_event);
                if (_event.type == SDL_QUIT)
                {
                    done = true;
                    m_controller->ShutModel();
                }
                if (_event.type == SDL_WINDOWEVENT && _event.window.event == SDL_WINDOWEVENT_CLOSE && _event.window.windowID == SDL_GetWindowID(window))
                {
                    done = true;
                    m_controller->ShutModel();
                }
                if (m_inputDone)
                    GetFrameEvents().clear();
            }
            m_renderDone = true;

            // Start the Dear ImGui frame
            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            // RENDER GUI HERE ***************

            Render_GUI();
            UI_CornerInfo(io);

            // RENDER GUI HERE ***************

            // Rendering
            ImGui::Render();
            SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
            SDL_SetRenderDrawColor(renderer, (Uint8)(m_clearColor.x * 255), (Uint8)(m_clearColor.y * 255), (Uint8)(m_clearColor.z * 255), (Uint8)(m_clearColor.w * 255));
            SDL_RenderClear(renderer);

            // RENDER OBJECTS HERE ***************

            CheckModelEvents();

            if (m_enableBackgroundAnimations)
                Render_BackgroundAnimations(renderer);

            Render_Model(renderer);

            if (m_enableAnimations)
                Render_Animations(renderer);

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

void View::Render_Polygon(SDL_Renderer *renderer, const std::vector<Point> &points, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
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
    if (m_controller != nullptr)
    {
        const std::vector<std::shared_ptr<Shape>> &shapeList = m_controller->RetrieveModel_ReadShapes();
        for (int i = 0; i < shapeList.size(); i++)
        {
            Render_Polygon(
                renderer,
                Utils::convertToPointCloud(shapeList[i]), m_objectColors[i][0], m_objectColors[i][1], m_objectColors[i][2], m_objectColors[i][3]);
        }
    }
}

void View::Render_Animations(SDL_Renderer *renderer)
{
    for (int i = 0; i < animations.size(); i++)
    {
        animations[i]->tick(renderer);
        if (animations[i]->timeElapsed > animations[i]->duration)
        {
            animations[i].reset();
            animations.erase(animations.begin() + i);
        }
    }
}

void View::Render_BackgroundAnimations(SDL_Renderer *renderer)
{
    for (int i = 0; i < backgroundAnimations.size(); i++)
    {
        backgroundAnimations[i]->tick(renderer);
        if (backgroundAnimations[i]->timeElapsed > backgroundAnimations[i]->duration)
        {
            backgroundAnimations[i].reset();
            backgroundAnimations.erase(animations.begin() + i);
        }
    }
}

void View::Render_SwapBackgroundAnimation(std::shared_ptr<Animation> backgroundAnimation)
{
    for (int i = 0; i < backgroundAnimations.size(); i++)
    {
        if (backgroundAnimations[i]->animationType == ANIMATION_TYPE::WALLPAPER)
        {
            backgroundAnimations[i].reset();
            backgroundAnimations.erase(backgroundAnimations.begin() + i);
        }
    }
    backgroundAnimations.push_back(backgroundAnimation);
}

void View::Render_GUI()
{
    UI_ConstructMenuModule();
    UI_Tutorial();
}

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

int View::UI_FetchID()
{
    return this->ImGuiID++;
}

void View::UI_Interactive_CommonShapeSubMenu()
{

    if (ImGui::CollapsingHeader("Add Common Shapes", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::ColorEdit3("Shape Color", (float *)&m_currentShapeColor);

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
    static int sides = 5;
    static float xVel = 750.0f;
    static float yVel = 250.0f;
    static float rot = 0.0f;
    static float mass = 1.0f;
    static ImVec4 errorColor = TELOS_IMGUI_CLEAR;
    static std::string errorText;

    ImGui::InputFloat(("Radius##ID" + std::to_string(UI_FetchID())).c_str(), &radius);
    ImGui::InputInt(("Sides##ID" + std::to_string(UI_FetchID())).c_str(), &sides);
    UI_StandardShapeInputs(xVel, yVel, rot, mass);

    if (ImGui::Button("Add RP"))
    {
        MODEL_MODIFICATION_RESULT s = m_controller->UpdateModel_AddShape_RegularPoly(radius, sides, xVel, yVel, rot, mass);
        if (!UI_ModelModError(s, errorText, errorColor))
        {
            AddRenderColor();
        }
    }
    ImGui::TextColored(errorColor, "%s", errorText.c_str());
}

void View::UI_Interactive_AddRectangleButton()
{
    ImGui::Text("Rectangle");
    static float w = 250, h = 50;
    static float xVel = 1000.0f;
    static float yVel = 2500.0f;
    static float rot = 0.5f;
    static float mass = 1.0f;
    static ImVec4 errorColor;
    static std::string errorText;

    ImGui::InputFloat(("Width##ID" + std::to_string(UI_FetchID())).c_str(), &w);
    ImGui::InputFloat(("Height##ID" + std::to_string(UI_FetchID())).c_str(), &h);
    UI_StandardShapeInputs(xVel, yVel, rot, mass);

    if (ImGui::Button("Add Rect"))
    {
        MODEL_MODIFICATION_RESULT status = m_controller->UpdateModel_AddShape_Rect(w, h, xVel, yVel, rot, mass);
        if (!UI_ModelModError(status, errorText, errorColor))
        {
            AddRenderColor();
        }
    }

    ImGui::TextColored(errorColor, "%s", errorText.c_str());
}

void View::UI_Interactive_AddArbPolygonInput()
{
    ImGui::Text("Arbitrary Shape");
    // 4500 = ( {4 digits}.{16 digits} , {4 digits}.{16 digits} ) x 100 + 99 + extra just in case (justin beiber haha)
    static char inputBuf[4500] = "(0,0),(100,50),(200,150),(250,250),(150,350),(50,300)";
    static float xVel = 69.0f;
    static float yVel = 69.0f;
    static float rot = 1.0f;
    static float mass = 1.0f;
    static ImVec4 invalidInputTxtColor = TELOS_IMGUI_CLEAR;
    static std::string errorText;

    ImGui::InputText(("Points##ID" + std::to_string(UI_FetchID())).c_str(), inputBuf, sizeof(inputBuf));
    UI_StandardShapeInputs(xVel, yVel, rot, mass);

    if (ImGui::Button("Add AS"))
    {
        MODEL_MODIFICATION_RESULT status = m_controller->UpdateModel_AddShape_Arbitrary(inputBuf, xVel, yVel, rot, mass);
        if (!UI_ModelModError(status, errorText, invalidInputTxtColor))
        {
            AddRenderColor();
        }
    }

    ImGui::TextColored(invalidInputTxtColor, "%s", errorText.c_str());
}

void View::UI_StandardShapeInputs(float &xVel, float &yVel, float &rot, float &mass)
{
    ImGui::InputFloat(("X Velocity##ID" + std::to_string(UI_FetchID())).c_str(), &xVel);
    ImGui::InputFloat(("Y Velocity##ID" + std::to_string(UI_FetchID())).c_str(), &yVel);
    ImGui::InputFloat(("Rotational Velocity##ID" + std::to_string(UI_FetchID())).c_str(), &rot);
    ImGui::InputFloat(("Mass##ID" + std::to_string(UI_FetchID())).c_str(), &mass);
}

void View::UI_About()
{
    if (ImGui::CollapsingHeader("About - Read me!", ImGuiTreeNodeFlags_CollapsingHeader))
    {

        ImGui::SetWindowFontScale(2.0f);
        ImGui::NewLine();
        ImGui::Text("How the engine works\n\n");
        ImGui::SetWindowFontScale(1.25f);

        static const char *explanation =
            "Most physics engines of this scope employ the same program loop, that being, in order, "
            "updating the objects positions, broad phase collision detection, narrow phase collision "
            "detection, and collision resolution. Collision detection is an expensive procedure, "
            "with the best algorithms for their use cases ranging anywhere from a time complexity of O(nlogn) "
            "to O(n^2).\n\n"

            "To combat this, broad phase collision detection involves eliminating all shapes that \"obviously\" "
            "won't collide with each other. In the case of Telos, a simple sweep-and-prune algorithm is used that projects "
            "the shapes onto the x-axis, where any shapes that do not overlap in their projections are definitely not colliding, "
            "and hence not considered. The remaining pairs of shapes, which may be colliding, are fed into the actual collision detection algorithm. "
            "Telos leverages the Separation-Axis Theorem (SAT), for general collision detection, "
            "then employs a simple line intersection algorithm to find the exact point of collision.\n\n"

            "Given the discrete nature of computation, two colliding shapes must overlap in order to be considered as "
            "touching. If in one frame two shapes are not colliding, and the next frame they are - the collision must "
            "have occured somewhere between these two frames. While normally numerical integration methods are used to find the "
            "exact time of collision (or rather, a very accurate approximation), Telos simply uses a binary search technique to "
            "back up the shapes by increasingly smaller fractions of time for a set number of iterations, where the ending time is "
            "taken as the time of the collision. While this is slightly less optimal than more robust integration methods, it does indeed "
            "work quite well in practice, and produces good results.\n\n"

            "Collision resolution involves applying the appropriate forces to the shapes to have them react as they would in real "
            "life given their motion. A physics engine doesn't typically apply a force in the traditional sense, but "
            "simply calculates what the trajectories and rotations are going to be after be and changes the velocities of the shape.\n\n"

            "As far as Telos is concerned, given that there is no friction or any other-worldly forces, calculating the new velocities "
            "of the shapes (both rotational and translational) is surprisingly easy. Simply put, the impulse generated by the collision "
            "can be calculated in a few steps, and we simply apply this impulse (which is nothing more than modifying the velocities "
            "with the impulse parameter) to both the shapes.\n\n"

            "The equation for the resulting translational velocities of the shapes is:\n\n";

        ImGui::TextWrapped("%s", explanation);
        ImGui::TextColored(TELOS_IMGUI_RED, "vf = vi + jn / m\n\n");

        static const char *explanation2 = "Where 'j' is the impulse parameter, 'n' is the normal of the collision surface. The equation for "
                                          "the resulting rotational velocities is:\n\n";
        ImGui::TextWrapped("%s", explanation2);
        ImGui::TextColored(TELOS_IMGUI_RED, "wf = wi + (r x jn) / I\n\n");

        static const char *explanation3 = "Where 'r' is the vector that points from the centroid of the "
                                          "shapes to the collision point, 'I' the rotational inertia of the shapes, and 'x' the cross product.\n\n\n";
        ImGui::TextWrapped("%s", explanation3);

        ImGui::SetWindowFontScale(2.0f);
        ImGui::TextWrapped("Engine Architecture\n\n");
        ImGui::SetWindowFontScale(1.25f);

        static const char *paragraphs2 =
            "Telos follows the standard Model-View-Controller (MVC) architecture. The view (what you're seeing now) interfaces with "
            "the model (the physics engine) through the controller. Multithreading is supported, with the model and view running in separate "
            "threads (the model runs on one thread, and the view is split into two threads - rendering and user input). If you'd like to "
            "see an architecture diagram, you may see it a the bottom of the front page of the engines source code at:\n\n";

        ImGui::PushItemWidth(ImGui::GetWindowWidth());
        ImGui::TextWrapped("%s", paragraphs2);
        char buf[] = "https://github.com/Argyraspides/Telos";
        ImGui::InputText("##ID", buf, 38);

#if BUILD_EMCC
        if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftCtrl)) && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)))
        {
            emscripten_browser_clipboard::copy(buf);
        }
#endif
        ImGui::SetWindowFontScale(1.0f);
        ImGui::NewLine();
    }
}

void View::UI_ConstructMenuModule()
{
    ImGui::Begin("Menu");

    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(m_menuWidth, ImGui::GetIO().DisplaySize.y));
    m_menuOpen = !ImGui::IsWindowCollapsed();

    UI_About();

    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5);

    UI_ModelInfo();
    UI_Interactive_CommonShapeSubMenu();
    UI_ShapeInfo();

    ImGui::End();
}

void View::UI_ModelInfo()
{
    static float e = 1.0f;
    static float wallE = 1.0f; // WWWWAAAAAALLLLLL-E
    static int wallColRes = 7;
    static int shapeColRes = 7;
    if (ImGui::CollapsingHeader("Engine Parameters", ImGuiTreeNodeFlags_CollapsingHeader))
    {
        ImGui::TextColored(TELOS_IMGUI_WHITE, "Engine time step: %.3fs", m_controller->RetrieveModel_GetTimeStep());
        ImGui::TextColored(TELOS_IMGUI_WHITE, "Time elapsed: %.3fs", m_controller->RetrieveModel_GetCurrentTime());

        ImGui::NewLine();

        ImGui::TextColored(TELOS_IMGUI_RED0, "Maximum allowed velocities: (%.3f, %.3f) px/s", m_controller->RetrieveModel_GetMaxVelocity().x, m_controller->RetrieveModel_GetMaxVelocity().y);
        ImGui::TextColored(TELOS_IMGUI_RED0, "Maximum allowed rotational velocity: %.3f rad/s", m_controller->RetrieveModel_GetMaxRotVelocity());
        ImGui::TextColored(TELOS_IMGUI_RED0, "Maximum allowed mass: %.3f kg", m_controller->RetrieveModel_GetMaxMass());
        ImGui::TextColored(TELOS_IMGUI_RED0, "Maximum allowed shapes: %i", m_controller->RetrieveModel_GetMaxObjects());
        ImGui::TextColored(TELOS_IMGUI_RED0, "Maximum energy conservation violation: %.10f Joules", m_controller->RetrieveModel_GetMaxEnergyViolation());
        ImGui::TextColored(TELOS_IMGUI_RED0, "Maximum shape vertices: %i", m_controller->RetrieveModel_GetMaxPCSPoints());

        ImGui::NewLine();

        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5); // Set the next added elements to this width

        ImGui::ColorEdit3("Static Background Color", (float *)&m_clearColor);
        if (ImGui::ColorEdit3("Gradient Background Color", (float *)&m_gradientClearColor))
        {
            BackgroundGradientAnimation bg(1, AnimationUtils::imGuiToSDLColor(m_gradientClearColor));
            std::shared_ptr<Animation> bgPtr = std::make_shared<BackgroundGradientAnimation>(bg);
            Render_SwapBackgroundAnimation(bgPtr);
        }

        if (ImGui::SliderFloat("Collision Elasticity", &e, m_controller->RetrieveModel_GetMinElasticity(), m_controller->RetrieveModel_GetMaxElasticity()))
            m_controller->UpdateModel_ChangeElasticity(e);
        if (ImGui::SliderFloat("Wall Collision Elasticity", &wallE, m_controller->RetrieveModel_GetMinElasticity(), m_controller->RetrieveModel_GetMaxElasticity()))
            m_controller->UpdateModel_ChangeWallElasticity(wallE);
        if (ImGui::SliderInt("Wall Collision Resolution", &wallColRes, m_controller->RetrieveModel_GetMinWallOverlapResolution(), m_controller->RetrieveModel_GetMaxWallOverlapResolution()))
            m_controller->UpdateModel_ChangeWallOverlapResolution(wallColRes);
        if (ImGui::SliderInt("Shape Collision Resolution", &shapeColRes, m_controller->RetrieveModel_GetMinShapeOverlapResolution(), m_controller->RetrieveModel_GetMaxShapeOverlapResolution()))
            m_controller->UpdateModel_ChangeShapeOverlapResolution(shapeColRes);

        ImGui::NewLine();

        ImGui::SliderInt("Collision Particle Size", &m_collisionParticleRadii, 1, 15);
        ImGui::SliderInt("Collision Animation Duration", &m_collisionParticleAnimationDuration, 1, 30);

        ImGui::NewLine();

        ImGui::Checkbox("Enable Animations", &m_enableAnimations);
        ImGui::Checkbox("Enable Background Animations", &m_enableBackgroundAnimations);

        ImGui::PopItemWidth(); // Restore default item width

        ImGui::PushStyleColor(ImGuiCol_Button, TELOS_IMGUI_RED);

        ImGui::NewLine();

        UI_DeleteAllShapesBtn();

        ImGui::NewLine();

        ImGui::PopStyleColor();
    }
}

void View::UI_DeleteAllShapesBtn()
{
    if (ImGui::Button("Delete All Shapes"))
    {
        m_controller->UpdateModel_RemoveAllShapes();
        m_objectColors.clear();
    }
}

void View::UI_CornerInfo(ImGuiIO &io)
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
    if (m_controller->CheckModel_IsPaused())
    {
        ImGui::SetNextWindowPos(ImVec2(SCREEN_WIDTH - 90, 50), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(90, 50), ImGuiCond_Always);
        ImGui::Begin("Paused", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
        ImVec2 textSize = ImGui::CalcTextSize("PAUSED");
        ImVec2 textPosition = ImVec2((ImGui::GetWindowWidth() - textSize.x) * 0.5f, (ImGui::GetWindowHeight() - textSize.y) * 0.5f);
        ImGui::SetCursorPos(textPosition);
        ImGui::TextColored(TELOS_IMGUI_RED, "PAUSED");
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
            "Welcome to Telos! A 2D rigid body physics engine\n\n"

            "While still incomplete, with some features to come,\n"
            "the engine is functional and you may play around with\n"
            "it as development progresses.\n\n"

            "Controls:\n\n"

            "Esc: Pause\n"
            "Left click hold: Drag shapes around (pauses the engine)\n"
            "Right click: Delete a shape\n"
            "Left and right arrows: move backward/forward by one time step (you must pause first)\n"
            "Left shift hold: Hover over a shape and fling it!\n\n"
            "You can turn off/configure animations in the 'Engine Parameters' menu";

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
        const std::vector<std::shared_ptr<Shape>> &shapeList = m_controller->RetrieveModel_ReadShapes();
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

bool View::UI_ModelModError(const MODEL_MODIFICATION_RESULT &s, std::string &errorText, ImVec4 &textColor)
{
    switch (s.currentStatus)
    {
    case MODEL_MODIFICATION_RESULT::PCS_ADD_FAIL_EXCEEDED_MAX_POINTS:
        textColor = TELOS_IMGUI_RED;
        errorText = "Exceeded maximum allowed sides\nCheck the engine parameters menu for max values";
        return true;

    case MODEL_MODIFICATION_RESULT::PCS_ADD_FAIL_INSUFFICIENT_POINTS:
        textColor = TELOS_IMGUI_RED;
        errorText = "A polygon can't have less than 3 sides!";
        return true;

    case MODEL_MODIFICATION_RESULT::PCS_ADD_FAIL_EXCEEDED_MAX_SHAPE_PARAMS:
        textColor = TELOS_IMGUI_RED;
        errorText = "Exceeded max/min velocity(ies) and/or mass\nCheck the engine parameters menu for max values";
        return true;

    case MODEL_MODIFICATION_RESULT::PCS_ADD_FAIL_INVALID_POINT_INPUT:
        textColor = TELOS_IMGUI_RED;
        errorText = "Invalid point input";
        return true;

    case MODEL_MODIFICATION_RESULT::PCS_ADD_FAIL_NOT_CONVEX:
        textColor = TELOS_IMGUI_RED;
        errorText = "Shape is not convex!";
        return true;

    default:
        textColor = TELOS_IMGUI_CLEAR;
        errorText = "";
        return false;
    }
}

void View::AddRenderColor()
{
    Uint8 r = (Uint8)(m_currentShapeColor.x * pixelLimit);
    Uint8 g = (Uint8)(m_currentShapeColor.y * pixelLimit);
    Uint8 b = (Uint8)(m_currentShapeColor.z * pixelLimit);
    Uint8 a = (Uint8)(m_currentShapeColor.w * pixelLimit);
    std::array<Uint8, 4> color = {r, g, b, a};
    m_objectColors.push_back(color);
}

void View::CheckModelEvents()
{
    if (m_enableAnimations)
    {
        const std::vector<ModelEvent> &modelEvents = m_controller->RetrieveModel_GetEvents();
        for (int i = 0; i < modelEvents.size(); i++)
        {
            if (modelEvents[i].collisionLocation != NULLPOINT)
            {
                ParticleExplosionAnimation s(modelEvents[i].collisionLocation, m_collisionParticleAnimationDuration);
                s.particleRadius = m_collisionParticleRadii;
                animations.push_back(std::make_shared<ParticleExplosionAnimation>(s));
            }
        }
    }
    m_controller->UpdateModel_ClearEvents();
}

void View::SDL_EventHandlingLoop()
{
    const std::chrono::milliseconds frameDuration(1000 / VIEW_INPUT_POLLING_RATE);
    auto startTime = std::chrono::high_resolution_clock::now();
    while (!done)
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        m_inputDone = false;
        if (elapsed >= frameDuration)
        {
            for (SDL_Event &event : GetFrameEvents())
            {
                SDL_ViewportHandler(event);
                if (m_renderDone)
                    GetFrameEvents().clear();
            }
            m_inputDone = true;
            startTime = std::chrono::high_resolution_clock::now();
        }
    }
}

void View::SDL_ViewportHandler(SDL_Event &event)
{
    SDL_DragShape(event);
    SDL_ThrowShape(event);
    SDL_RemoveShape(event);
    SDL_Pause(event);
    SDL_TickModel(event);
}

void View::SDL_DragShape(SDL_Event &event)
{
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    if (m_menuOpen && mouseX < m_menuWidth)
        return;

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
    {
        const std::vector<std::shared_ptr<Shape>> &shapePtrs = m_controller->RetrieveModel_ReadShapes();
        for (std::shared_ptr<Shape> shapePtr : shapePtrs)
        {

            if (Utils::isInside({(double)mouseX, (double)mouseY}, shapePtr))
            {
                m_controller->PauseModel();
                while (true)
                {
                    SDL_PollEvent(&event);
                    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
                    {
                        break;
                    }
                    SDL_GetMouseState(&mouseX, &mouseY);
                    shapePtr->setShapePos(Point({(float)mouseX, (float)mouseY, 0}));
                }
            }
        }
    }
}

void View::SDL_ThrowShape(SDL_Event &event)
{
    int mouseX, mouseY;
    int initMouseX, initMouseY;
    SDL_GetMouseState(&initMouseX, &initMouseY);

    if (m_menuOpen && initMouseX < m_menuWidth)
        return;

    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LSHIFT)
    {
        const std::vector<std::shared_ptr<Shape>> &shapePtrs = m_controller->RetrieveModel_ReadShapes();
        for (std::shared_ptr<Shape> shapePtr : shapePtrs)
        {
            if (Utils::isInside({(double)initMouseX, (double)initMouseY}, shapePtr))
            {
                m_controller->PauseModel();
                shapePtr->setShapeVel({0, 0, 0});
                while (true)
                {
                    SDL_PollEvent(&event);
                    SDL_GetMouseState(&mouseX, &mouseY);

                    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_LSHIFT)
                    {
                        Point vec = {(double)(mouseX - initMouseX) / 10.0, (double)(mouseY - initMouseY) / 10.0, 0};
                        shapePtr->setShapeVel(vec);
                        m_controller->UnpauseModel();
                        break;
                    }
                    shapePtr->setShapePos({(double)mouseX, (double)mouseY});
                }
            }
        }
    }
}

void View::SDL_RemoveShape(SDL_Event &event)
{
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    if (m_menuOpen && mouseX < m_menuWidth)
        return;
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT)
    {
        const std::vector<std::shared_ptr<Shape>> &shapePtrs = m_controller->RetrieveModel_ReadShapes();
        for (std::shared_ptr<Shape> shapePtr : shapePtrs)
        {
            if (Utils::isInside({(float)mouseX, (float)mouseY}, shapePtr))
            {
                m_objectColors.erase(m_objectColors.begin() + (int)shapePtr->getShapeID());
                m_controller->UpdateModel_RemoveShape(shapePtr);
                break;
            }
        }
    }
}

void View::SDL_Pause(SDL_Event &event)
{
    static ImVec4 userBackgroundCol;
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
    {
        if (!m_controller->CheckModel_IsPaused())
        {
            m_controller->PauseModel();
        }
        else
        {
            m_controller->UnpauseModel();
        }
    }
}

void View::SDL_TickModel(SDL_Event &event)
{
    if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d))
    {
        m_controller->UpdateModel_ForwardTick();
    }
    else if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a))
    {
        m_controller->UpdateModel_BackwardTick();
    }
}
