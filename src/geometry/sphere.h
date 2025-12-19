#pragma once

#include <glm/glm.hpp>

#include "material/material.h"
#include "accelerator/hittable.h"
#include "core/types.h"

namespace hasmet {
class Sphere : public Hittable {
 public:
  Sphere(const Vec3& center, float radius);

  virtual bool intersect(Ray& r, HitRecord& rec) const override;
  virtual AABB get_aabb() const override;

 private:
  AABB local_aabb_;
  Vec3 center_;
  float radius_;
};
} // namespace hasmet