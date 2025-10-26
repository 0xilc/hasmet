#pragma once

#include <glm/glm.hpp>
#include <limits>

struct Ray {
  glm::vec3 origin;
  glm::vec3 direction;

  float tmin = 1e-4f;
  float tmax = std::numeric_limits<float>::infinity();

  Ray() = default;
  Ray(const glm::vec3& o, const glm::vec3& d) : origin(o), direction(d) {}

  glm::vec3 at(float t) const { return origin + t * direction; }
};