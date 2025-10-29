#pragma once

#include "core/aabb.h"
#include "core/interval.h"
#include "core/ray.h"
#include "core/hit_record.h"

class Hittable {
 public:
  virtual ~Hittable() = default;

  virtual bool intersect(Ray& ray, HitRecord& rec) const = 0;
  virtual AABB getAABB() const = 0;
};
