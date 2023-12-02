#include "imgui.h"
#include "shape.h"

class Controller
{

public:

    Controller();

    void addShape();
    void removeShape();
    void changeShape();
    void changeGravity();
    void changeWallElasticity();

    void CommonShapeSubMenu();
    void CircleButton();
    void FullMenu();
    std::vector<Point> ResolveShapeDefinition(const Shape &shape);
};