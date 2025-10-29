#pragma once

#include <glm/glm.hpp>
#include <limits>
#include "interval.h"

struct Ray {
  glm::vec3 origin;
  glm::vec3 direction;

  Interval interval_ = Interval(0.004f, INFINITY);


  Ray() = default;
  Ray(const glm::vec3& o, const glm::vec3& d) : origin(o), direction(d) {}

  glm::vec3 at(float t) const { return origin + t * direction; }
};