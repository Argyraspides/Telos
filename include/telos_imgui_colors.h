#pragma once
#include "imgui.h"

// Colors with a higher number (e.g. LIGHTGRAY2, LIGHTGRAY3) are lighter, and vice versa

const float pixelLimit = 255.0f;

extern ImVec4 TELOS_IMGUI_WHITE;
extern ImVec4 TELOS_IMGUI_CLEAR;

extern ImVec4 TELOS_IMGUI_RED0;
extern ImVec4 TELOS_IMGUI_RED;

extern ImVec4 TELOS_IMGUI_GREEN;
extern ImVec4 TELOS_IMGUI_BLUE;
extern ImVec4 TELOS_IMGUI_YELLOW;
extern ImVec4 TELOS_IMGUI_PURPLE;
extern ImVec4 TELOS_IMGUI_LIGHTBLUE;
extern ImVec4 TELOS_IMGUI_DARKGRAY;
extern ImVec4 TELOS_IMGUI_LIGHTGRAY;
extern ImVec4 TELOS_IMGUI_LIGHTGRAY2;

namespace TELOS_IMGUI_COLORS
{
    ImVec4 getOppositeColor(ImVec4 color);
}
