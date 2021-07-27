#pragma once

#include "../precompiled.h"
#include "../config.h"

#include "detect-edge.hpp"
#include "edged-image.hpp"
#include "mat-to-texture.hpp"
#include "window.hpp"

std::optional<EdgedImage> editImageEdges(EdgedImage &image);
