#pragma once

#include "core/spectrum.h"
#include <vector>
#include <string>

bool write_png(const std::string& filename, const std::vector<Spectrum>& pixels,
               int width, int height);