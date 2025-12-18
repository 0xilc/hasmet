#pragma once
#include <glm/glm.hpp>

#include "accelerator/hittable.h"
namespace hasmet {
class Plane : public Hittable {
 public:
  Plane(const Vec3& center, const Vec3& normal, int material_id);

  virtual bool local_intersect(Ray& ray, HitRecord& rec) const override;
  virtual AABB get_aabb() const override;

 private:
  Vec3 center_;
  Vec3 normal_;
  int material_id_;
};
} // namespace hasmet