#pragma once
#include "core/types.h"

namespace hasmet {
struct AmbientLight {
  Color radiance;
  AmbientLight(const Color& rad) : radiance(rad) {}
};
} // namespace hasmet