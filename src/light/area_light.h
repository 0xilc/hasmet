#pragma once
#include "core/color.h"
#include "light.h"
#include <glm/glm.hpp>

struct AreaLight : public Light {
  glm::vec3 position;
  glm::vec3 normal;
  float size;
  Color radiance;
  glm::vec3 u;
  glm::vec3 v;

  AreaLight(const glm::vec3& pos, const glm::vec3& normal, float size,
            const Color& rad)
      : position(pos), normal(normal), size(size), radiance(rad) {
    
    // Generate orhonormal basis of the surface for sampling
    glm::vec3 w = glm::normalize(normal);
    glm::vec3 tmp = (std::abs(w.x) > 0.9f) ? glm::vec3(0.0f, 1.0f, 0.0f)
                                           : glm::vec3(1.0f, 0.0f, 0.0f);
    u = glm::normalize(glm::cross(tmp, w));
    v = glm::cross(w, u);
  }
};