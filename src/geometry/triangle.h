#pragma once
#include <glm/glm.hpp>

#include "accelerator/hittable.h"
#include "core/types.h"

namespace hasmet {
class Triangle : public Hittable {
 public:
  Triangle(const Vec3 verteices[3] = nullptr,
           const Vec3 vertex_normals[3] = nullptr,
           const Vec2 tex_coords[3] = nullptr,
           bool smooth_shading = false);

  virtual bool intersect(Ray& ray, HitRecord& rec) const override;
  virtual AABB get_aabb() const override;

 private:
  AABB local_aabb_;
  Vec3 vertices_[3];
  Vec3 vertex_normals_[3];
  Vec2 tex_coords_[3];
  bool smooth_shading_;
};
} // namespace hasmet 