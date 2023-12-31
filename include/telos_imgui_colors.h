#pragma once
#include "imgui.h"

const float pixelLimit = 255.0f;

auto TELOS_IMGUI_WHITE = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
auto TELOS_IMGUI_CLEAR = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

auto TELOS_IMGUI_RED = ImVec4(255.0f / pixelLimit, 45.0f / pixelLimit, 0.0f / pixelLimit, 1.0f);
auto TELOS_IMGUI_GREEN = ImVec4(20.0f / pixelLimit, 255.0f / pixelLimit, 0.0f / pixelLimit, 1.0f);
auto TELOS_IMGUI_BLUE = ImVec4(0.0f / pixelLimit, 85.0f / pixelLimit, 255.0f / pixelLimit, 1.0f);
auto TELOS_IMGUI_YELLOW = ImVec4(241.0f / pixelLimit, 255.0f / pixelLimit, 0.0f / pixelLimit, 1.0f);
auto TELOS_IMGUI_PURPLE = ImVec4(192.0f / pixelLimit, 0.0f / pixelLimit, 255.0f / pixelLimit, 1.0f);
auto TELOS_IMGUI_LIGHTBLUE = ImVec4(0.0f / pixelLimit, 190.0f / pixelLimit, 255.0f / pixelLimit, 1.0f);