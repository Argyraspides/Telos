#include "model.h"

std::vector<Point> pts = PointCloudShape_Cvx::generateCircle(100);
std::vector<PointCloudShape_Cvx> Model::pointCloudShapeList = {PointCloudShape_Cvx(pts)};

Model::Model()
{

}
