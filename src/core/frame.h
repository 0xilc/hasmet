#pragma once

#include "core/types.h"

namespace hasmet {

struct Frame {
  Vec3 tangent, bitangent, normal;

  Frame(const Vec3 &n) : normal(n) {
    Vec3 up = std::abs(normal.z) < 0.999f ? Vec3(0, 0, 1) : Vec3(1, 0, 0);
    tangent = glm::normalize(glm::cross(up, normal));
    bitangent = glm::cross(normal, tangent);
  }

  Vec3 to_local(const Vec3 &v) const {
    return Vec3(glm::dot(v, tangent), glm::dot(v, bitangent),
                glm::dot(v, normal));
  }

  Vec3 to_world(const Vec3 &v) const {
    return tangent * v.x + bitangent * v.y + normal * v.z;
  }
};

} // namespace hasmet