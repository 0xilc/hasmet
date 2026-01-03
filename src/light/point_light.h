#pragma once
#include "light.h"

#include "core/types.h"

namespace hasmet {
struct PointLight : public Light {
  Vec3 position;
  Color intensity;

  PointLight(const Vec3& pos, const Color& i)
      : position(pos), intensity(i) {}
  
  LightSample sample_li(const Vec3& hit_point, const Vec2& u) const;
};
} // namespace hasmet