#pragma once

#include "shape.h"
#include "core/vec.h"

class Sphere : public Shape {
 public:
  Sphere(const Vec3& center, float radius);
  
  virtual bool intersect(const Ray& r, HitRecord& rec) const override;

 private:
  Vec3 center_;
  float radius_;
};