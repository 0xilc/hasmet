#pragma once

#include <glm/glm.hpp>
#include <limits>
#include "interval.h"
#include "core/sampling.h"
#include "core/types.h"

namespace hasmet {
struct Ray {
  Vec3 origin;
  Vec3 direction;
  float time;
  float t_min = 0.001f;
  float t_max = std::numeric_limits<float>::infinity();

  Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d) {}

  Vec3 at(float t) const { return origin + t * direction; }
};
} // namespace hasmet