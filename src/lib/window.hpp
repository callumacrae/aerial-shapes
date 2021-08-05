#include <cmath>

#include "../precompiled.h"

using onFrameFn = std::function<bool(GLFWwindow*, ImGuiIO&)>;

void initWindow(int width, int height, const char* title);
void openWindow(const onFrameFn &onFrame);

const ImGuiWindowFlags staticWindowFlags =
    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
    ImGuiWindowFlags_NoSavedSettings;
