#pragma once

#include "core/aabb.h"
#include "core/hit_record.h"
#include "core/interval.h"
#include "core/ray.h"
#include "core/types.h"

namespace hasmet{
struct SurfaceSample {
  Vec3 p;
  Vec3 n;
  float pdf = 0.0f;
};

class Hittable {
 public:
  virtual ~Hittable() = default;
  virtual bool intersect(Ray& ray, HitRecord& rec) const = 0;
  virtual AABB get_aabb() const = 0;
  virtual SurfaceSample sample_surface(const Vec2& u) const { return {}; }
  virtual float get_area() const { return 0.0f; }
};
} // namespace hasmet