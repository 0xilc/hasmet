#pragma once
#include <glm/glm.hpp>

#include "accelerator/hittable.h"

class Plane : public Hittable {
 public:
  Plane(const glm::vec3& center, const glm::vec3& normal, int material_id);

  virtual bool intersect(Ray& ray, HitRecord& rec) const override;
  virtual AABB getAABB() const override;

 private:
  glm::vec3 center_;
  glm::vec3 normal_;
  int material_id_;
  AABB aabb_;
};
