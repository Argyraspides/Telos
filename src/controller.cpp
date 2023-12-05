#include "controller.h"
#include "view.h"
#include "model.h"
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>

Controller::Controller(Model *model)
{
    this->model = model;
}

void Controller::addPointCloudShape(std::vector<Point> points, Point offset)
{
    for (Point &p : points)
        p = p + offset;

    PointCloudShape_Cvx pcs(points);
    std::shared_ptr<Shape> shape = std::make_shared<PointCloudShape_Cvx>(pcs);
    this->model->addShape(shape);
}

void Controller::moveShape(int shapeID)
{
}

// For any shape, resolves what kind of shape it is and then translates it into a point cloud for rendering
std::vector<Point> Controller::ResolveShapeDefinition(std::shared_ptr<Shape> shape)
{
    int shapeTypeID = shape->getShapeTypeID();
    if (shapeTypeID == SHAPE_TYPE_IDENTIFIERS::POINT_CLOUD_SHAPE_CVX)
    {
        std::shared_ptr<PointCloudShape_Cvx> pointCloudShape_Cvx = std::dynamic_pointer_cast<PointCloudShape_Cvx>(shape);
        return pointCloudShape_Cvx->getPoints();
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
    ImGui::Text("Circle");
    static float radius = 10;
    ImGui::InputFloat("Radius", &radius);

    if (ImGui::Button("Add"))
    {
        addPointCloudShape(PointCloudShape_Cvx::generateCircle(radius), {SCREEN_WIDTH / 2.0F, SCREEN_HEIGHT / 2.0F});
    }
}

void Controller::FullMenu()
{
    ImGui::Begin("Menu");
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(0.2 * SCREEN_WIDTH, ImGui::GetIO().DisplaySize.y));
    CommonShapeSubMenu();
}
