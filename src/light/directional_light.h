#pragma once
#include "light.h"

#include "core/types.h"

namespace hasmet {
struct DirectionalLight : public Light {
  Vec3 direction;
  Color radiance;

  DirectionalLight(const Vec3& direction, const Color& radiance)
      : direction(direction), radiance(radiance) {}
};
} // namespace hasmet