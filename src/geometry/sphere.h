#pragma once

#include <glm/glm.hpp>

#include "material/material.h"
#include "accelerator/hittable.h"
#include "core/types.h"

namespace hasmet {
class Sphere : public Hittable {
 public:
  Sphere(const Vec3& center, float radius, int material_id, Vec3 motion_blur);

  virtual bool local_intersect(Ray& r, HitRecord& rec) const override;
  virtual AABB get_aabb() const override;

 private:
  Vec3 center_;
  float radius_;
  int material_id_;
};
} // namespace hasmet