#pragma once

#include "core/color.h"
#include <vector>
#include <string>

bool write_png(const std::string& filename, const std::vector<Color>& pixels,
               int width, int height);