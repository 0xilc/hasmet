#pragma once
#include "light.h"
#include "core/types.h"

namespace hasmet {
struct AmbientLight : public Light {
  Color radiance;

  AmbientLight(const Color& rad) : radiance(rad) {}
};
} // namespace hasmet