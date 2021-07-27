#include <cmath>

#include "../precompiled.h"

using onFrameFn = std::function<bool(GLFWwindow*)>;

void initWindow(int width, int height, const char* title);
void openWindow(const onFrameFn onFrame);
