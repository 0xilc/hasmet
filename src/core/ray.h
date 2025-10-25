#pragma once

#include "vec.h"
#include <limits>

struct Ray {
  Vec3 origin;
  Vec3 direction;

  float tmin = 1e-4f;
  float tmax = std::numeric_limits<float>::infinity();

  Ray() = default;
  Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d) {}

  Vec3 at(float t) const { return origin + t * direction; }
};