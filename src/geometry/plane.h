#pragma once
#include <glm/glm.hpp>

#include "accelerator/hittable.h"

class Plane : public Hittable {
 public:
  Plane(const glm::vec3& center, const glm::vec3& normal, int material_id);

  virtual bool local_intersect(Ray& ray, HitRecord& rec) const override;
  virtual AABB get_aabb() const override;

 private:
  glm::vec3 center_;
  glm::vec3 normal_;
  int material_id_;
};
