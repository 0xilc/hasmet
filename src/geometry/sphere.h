#pragma once

#include "shape.h"
#include <glm/glm.hpp>

class Sphere : public Shape {
 public:
  Sphere(const glm::vec3& center, float radius);
  
  virtual bool intersect(const Ray& r, HitRecord& rec) const override;

 private:
  glm::vec3 center_;
  float radius_;
};