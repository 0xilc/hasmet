#pragma once

#include "light.h"
#include <glm/glm.hpp>
#include "core/types.h"

namespace hasmet {
struct AreaLight : public Light {
  Vec3 position;
  Vec3 normal;
  float size;
  Color radiance;
  Vec3 u;
  Vec3 v;

  AreaLight(const Vec3& pos, const Vec3& normal, float size,
            const Color& rad)
      : position(pos), normal(normal), size(size), radiance(rad) {
    
    // Generate orhonormal basis of the surface for sampling
    Vec3 w = glm::normalize(normal);
    Vec3 tmp = (std::abs(w.x) > 0.9f) ? Vec3(0.0f, 1.0f, 0.0f)
                                           : Vec3(1.0f, 0.0f, 0.0f);
    u = glm::normalize(glm::cross(tmp, w));
    v = glm::cross(w, u);
  }
};

} // namespace hasmet