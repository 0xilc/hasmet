#pragma once

#include <glm/glm.hpp>

#include "material/material.h"
#include "accelerator/hittable.h"

class Sphere : public Hittable {
 public:
  Sphere(const glm::vec3& center, float radius, int material_id);

  virtual bool intersect(Ray& r, HitRecord& rec) const override;
  virtual AABB getAABB() const override;

 private:
  glm::vec3 center_;
  float radius_;
  int material_id_;
  AABB aabb_;
};