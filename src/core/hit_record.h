#pragma once

#include <glm/glm.hpp>
#include "ray.h"

struct HitRecord {
  float t;
  glm::vec3 p;
  glm::vec3 normal;
  bool front_face;
  int material_id;

  inline void set_face_normal(const Ray& r, const glm::vec3& outward_normal) {
    front_face = glm::dot(r.direction, outward_normal);
    normal = front_face ? outward_normal : -outward_normal;
  }
};
