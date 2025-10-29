#pragma once
#include <glm/glm.hpp>

#include "shape.h"

class Triangle : Shape {
 public:
  Triangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, int material_id);
  
  virtual bool intersect(Ray& ray, HitRecord& rec) const override;
  virtual AABB getAABB() const override;

 private:
  glm::vec3 indices_[3];
  int material_id_;
  AABB aabb_;
};
