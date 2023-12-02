#include "controller.h"
#include "view.h"
#include <functional>
#include <iostream>
#include <memory>

Controller::Controller()
{
    
}

void Controller::addShape()
{
    
}


std::vector<Point> Controller::ResolveShapeDefinition(const Shape &shape)
{
    int shapeID = shape.getID();
    if (shapeID == POINT_CLOUD_SHAPE_CVX)
    {
        const PointCloudShape_Cvx &pointCloudShape_Cvx = dynamic_cast<const PointCloudShape_Cvx &>(shape);
        return pointCloudShape_Cvx.getPoints();
    }
    else
    {
        std::cerr << "SHAPE TYPE IS INVALID (FUNCTION View::ResolveShapeDefinition(const Shape &shape))" << std::endl;
    }
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

    ImGui::InputText("##TextEntry", textBuffer, sizeof(textBuffer));
    ImGui::SameLine();

    if (ImGui::Button("Add"))
    {
    }
}

void Controller::FullMenu()
{
    ImGui::Begin("Menu");
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(0.2 * SCREEN_WIDTH, ImGui::GetIO().DisplaySize.y));
    CommonShapeSubMenu();




}