#pragma once
#include <glm/glm.hpp>

#include "accelerator/hittable.h"

class Triangle : public Hittable {
 public:
  Triangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
           int material_id, const glm::vec3 vertex_normals[3] = nullptr, bool smooth_shading = false);

  virtual bool intersect(Ray& ray, HitRecord& rec) const override;
  virtual AABB getAABB() const override;

 private:
  glm::vec3 indices_[3];
  glm::vec3 vertex_normals_[3];
  int material_id_;
  AABB aabb_;
  bool smooth_shading_;
};
