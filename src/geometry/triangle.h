#pragma once
#include <glm/glm.hpp>

#include "accelerator/hittable.h"

class Triangle : public Hittable {
 public:
  Triangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, int material_id);
  
  virtual bool intersect(Ray& ray, HitRecord& rec) const override;
  virtual AABB getAABB() const override;

 private:
  glm::vec3 indices_[3];
  int material_id_;
  AABB aabb_;
};
