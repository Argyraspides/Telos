#include "imgui.h"
#include "telos_imgui_colors.h"
// Colors with a higher number (e.g. LIGHTGRAY2, LIGHTGRAY3) are lighter, and vice versa

ImVec4 TELOS_IMGUI_WHITE = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImVec4 TELOS_IMGUI_CLEAR = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

ImVec4 TELOS_IMGUI_RED0 = ImVec4(145.0f / pixelLimit, 27.0f / pixelLimit, 0.0f / pixelLimit, 1.0f);
ImVec4 TELOS_IMGUI_RED = ImVec4(255.0f / pixelLimit, 45.0f / pixelLimit, 0.0f / pixelLimit, 1.0f);

ImVec4 TELOS_IMGUI_GREEN = ImVec4(20.0f / pixelLimit, 255.0f / pixelLimit, 0.0f / pixelLimit, 1.0f);
ImVec4 TELOS_IMGUI_BLUE = ImVec4(0.0f / pixelLimit, 85.0f / pixelLimit, 255.0f / pixelLimit, 1.0f);
ImVec4 TELOS_IMGUI_YELLOW = ImVec4(241.0f / pixelLimit, 255.0f / pixelLimit, 0.0f / pixelLimit, 1.0f);
ImVec4 TELOS_IMGUI_PURPLE = ImVec4(192.0f / pixelLimit, 0.0f / pixelLimit, 255.0f / pixelLimit, 1.0f);
ImVec4 TELOS_IMGUI_LIGHTBLUE = ImVec4(0.0f / pixelLimit, 190.0f / pixelLimit, 255.0f / pixelLimit, 1.0f);
ImVec4 TELOS_IMGUI_DARKGRAY = ImVec4(30.0f / pixelLimit, 30.0f / pixelLimit, 30.0f / pixelLimit, 30.0f / pixelLimit);
ImVec4 TELOS_IMGUI_LIGHTGRAY = ImVec4(90.0f / pixelLimit, 90.0f / pixelLimit, 90.0f / pixelLimit, 90.0f / pixelLimit);
ImVec4 TELOS_IMGUI_LIGHTGRAY2 = ImVec4(180.0f / pixelLimit, 180.0f / pixelLimit, 180.0f / pixelLimit, 180.0f / pixelLimit);

ImVec4 TELOS_IMGUI_COLORS::getOppositeColor(ImVec4 color)
{
    float r = (255.0 - (255.0 * color.x)) / 255.0;
    float g = (255.0 - (255.0 * color.y)) / 255.0;
    float b = (255.0 - (255.0 * color.z)) / 255.0;
    float a = (255.0 - (255.0 * color.w)) / 255.0;

    return ImVec4(r, g, b, a);
}