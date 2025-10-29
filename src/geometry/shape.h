#pragma once

#include "core/ray.h"
#include "accelerator/hittable.h"

class Shape {
 public:
  virtual ~Shape() = default;
  virtual bool intersect(const Ray& r, HitRecord& rec) const = 0;
};