#pragma once
#include "light.h"

#include "core/types.h"
#include "core/hit_record.h"

namespace hasmet {
struct DirectionalLight : public Light {
  Vec3 direction;
  Color radiance;

  DirectionalLight(const Vec3& direction, const Color& radiance)
      : direction(direction), radiance(radiance) {}
  
  LightSample sample_li(const HitRecord& rec, const Vec2& u) const override;
};
} // namespace hasmet