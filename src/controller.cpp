#include "controller.h"
#include "view.h"
#include "model.h"
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>

Controller::Controller()
{
}

void Controller::addPointCloudShape(const std::vector<Point> &points)
{
}

// For any shape, resolves what kind of shape it is and then translates it into a point cloud for rendering
std::vector<Point> Controller::ResolveShapeDefinition(const Shape &shape)
{
    int shapeID = shape.getID();
    if (shapeID == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        const PointCloudShape_Cvx &pointCloudShape_Cvx = dynamic_cast<const PointCloudShape_Cvx &>(shape);
        return pointCloudShape_Cvx.getPoints();
    }

    std::cerr << "SHAPE TYPE IS INVALID (FUNCTION View::ResolveShapeDefinition(const Shape &shape))" << std::endl;
    return {};
}

void Controller::CommonShapeSubMenu()
{

    if (ImGui::CollapsingHeader("Add Common Shapes", ImGuiTreeNodeFlags_DefaultOpen))
    {
        CircleButton();
    }

    ImGui::End();
}

void Controller::CircleButton()
{
    char textBuffer[256] = "";
    ImGui::Text("Circle");

    static float radius = 10;
    ImGui::InputFloat("Radius", &radius);

    if (ImGui::Button("Add"))
    {
        std::vector<Point> pts = PointCloudShape_Cvx::generateCircle(radius);
        Model::pointCloudShapeList.push_back(PointCloudShape_Cvx(pts));
    }
}

void Controller::FullMenu()
{
    ImGui::Begin("Menu");
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(0.2 * SCREEN_WIDTH, ImGui::GetIO().DisplaySize.y));
    CommonShapeSubMenu();
}
