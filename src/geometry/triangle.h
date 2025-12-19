#pragma once
#include <glm/glm.hpp>

#include "accelerator/hittable.h"
#include "core/types.h"

namespace hasmet {
class Triangle : public Hittable {
 public:
  Triangle(const Vec3& p1, const Vec3& p2, const Vec3& p3,
           const Vec3 vertex_normals[3] = nullptr,
           bool smooth_shading = false);

  virtual bool intersect(Ray& ray, HitRecord& rec) const override;
  virtual AABB get_aabb() const override;

 private:
  AABB local_aabb_;
  Vec3 indices_[3];
  Vec3 vertex_normals_[3];
  bool smooth_shading_;
};
} // namespace hasmet 