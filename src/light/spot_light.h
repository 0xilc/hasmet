#pragma once
#include "light.h"

#include "core/types.h"

namespace hasmet {
struct SpotLight : public Light {
  Vec3 position;
  Vec3 direction;
  Vec3 intensity;
  float coverage_angle;
  float falloff_angle;

  SpotLight(const Vec3& position, const Vec3& direction, const Vec3& intensity, float coverage, float falloff)
    : position(position), 
      direction(glm::normalize(direction)), 
      intensity(intensity), 
      coverage_angle(coverage), 
      falloff_angle(falloff) 
  {}

  LightSample sample_li(const Vec3& hit_point, const Vec2& u) const;
};
} // namespace hasmet