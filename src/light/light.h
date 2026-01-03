#pragma once

#include "core/types.h"

namespace hasmet {
struct LightSample {
  Color L;
  Vec3 wi;
  float pdf;
  float dist;
};

class Light{
 public:
  // virtual LightSample sample_li(const Vec3& hit_point, const Vec2& uxuy) const = 0;
};
} // namespace hasmet