#pragma once
#include "imgui.h"
#include "model.h"

class Controller
{
public:
    Model *m_model;
    Controller(Model *model);
};