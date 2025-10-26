#pragma once

#include <glm/glm.hpp>

#include "material/material.h"
#include "shape.h"

class Sphere : public Shape {
 public:
  Sphere(const glm::vec3& center, float radius, Material* material);

  virtual bool intersect(const Ray& r, HitRecord& rec) const override;

 private:
  glm::vec3 center_;
  float radius_;
  Material* material_;
};