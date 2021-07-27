#include "../precompiled.h"

using onFrameFn = std::function<bool(void)>;

void initWindow(int width, int height, const char* title);
void openWindow(const onFrameFn onFrame);
