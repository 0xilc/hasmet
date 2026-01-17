#pragma once

#include <glm/glm.hpp>
#include "ray.h"
#include "core/types.h"
#include <optional>

namespace hasmet {
struct HitRecord {
  float t;
  Vec3 p;
  Vec3 normal;
  Vec3 tangents[2];
  bool front_face;
  int material_id;
  Vec2 uv{0.0f, 0.0f};
  const std::vector<int>* texture_ids = nullptr;
  std::optional<Color> radiance;
  
  inline void set_face_normal(const Ray& r, const Vec3& outward_normal) {
    front_face = glm::dot(r.direction, outward_normal);
    normal = front_face ? outward_normal : -outward_normal;
  }

  inline Color Le(Vec3 wo) {
    if (glm::dot(wo, normal) > 0) {
      return radiance.value();
    }
    
    return Color(0.0f);
  }
};
} // namespace hasmet