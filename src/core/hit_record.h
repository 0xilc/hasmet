#pragma once

#include <glm/glm.hpp>
#include "ray.h"
#include "core/types.h"

namespace hasmet {
struct HitRecord {
  float t;
  Vec3 p;
  Vec3 normal;
  bool front_face;
  int material_id;
  Vec2 uv{0.0f, 0.0f};

  inline void set_face_normal(const Ray& r, const Vec3& outward_normal) {
    front_face = glm::dot(r.direction, outward_normal);
    normal = front_face ? outward_normal : -outward_normal;
  }
};
} // namespace hasmet