#pragma once

#include "core/ray.h"
#include "accelerator/hittable.h"

class Shape {
 public:
  virtual ~Shape() = default;
  virtual bool intersect(Ray& r, HitRecord& rec) const = 0;
  virtual AABB getAABB() const = 0;

};