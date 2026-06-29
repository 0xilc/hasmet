#pragma once

#include "core/hit_record.h"
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
  virtual ~Light() = default;
  virtual LightSample sample_li(const HitRecord& rec, const Vec2& u) const = 0;
  virtual float pdf_li(const HitRecord& rec, const Vec3& wi) const { return 0.0f; }
};
} // namespace hasmet