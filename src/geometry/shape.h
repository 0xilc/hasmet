#pragma once

#include "core/ray.h"
#include "core/vec.h"

struct HitRecord {
  float t;
  Vec3 p;
  Vec3 normal;
  bool front_face;

  inline void set_face_normal(const Ray& r, const Vec3& outward_normal) {
    front_face = dot(r.direction, outward_normal);
    normal = front_face ? outward_normal : -outward_normal;
  }
};

class Shape {
 public:
  virtual ~Shape() = default;
  virtual bool intersect(const Ray& r, HitRecord& rec) const = 0;
};