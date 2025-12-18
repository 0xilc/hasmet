#pragma once
#include <glm/glm.hpp>

#include "accelerator/hittable.h"
#include "core/types.h"

namespace hasmet {
class Triangle : public Hittable {
 public:
  Triangle(const Vec3& p1, const Vec3& p2, const Vec3& p3,
           int material_id, const Vec3 vertex_normals[3] = nullptr,
           bool smooth_shading = false);

  virtual bool local_intersect(Ray& ray, HitRecord& rec) const override;
  virtual AABB get_aabb() const override;

 private:
  Vec3 indices_[3];
  Vec3 vertex_normals_[3];
  int material_id_;
  bool smooth_shading_;
};
} // namespace hasmet 