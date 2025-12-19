#pragma once

#include "core/aabb.h"
#include "core/hit_record.h"
#include "core/interval.h"
#include "core/ray.h"
#include "core/types.h"

namespace hasmet{
class Hittable {
 public:
  virtual ~Hittable() = default;
  virtual bool intersect(Ray& ray, HitRecord& rec) const = 0;
  virtual AABB get_aabb() const = 0;
};
} // namespace hasmet