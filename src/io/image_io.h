#pragma once


#include <vector>
#include <string>
#include "core/types.h"

namespace hasmet {
bool write_png(const std::string& filename, const std::vector<Color>& pixels,
               int width, int height);

} // namespace hasmet