#pragma once

#include <glm/glm.hpp>
#include <limits>
#include "interval.h"
#include "core/sampling.h"
#include "core/types.h"

namespace hasmet {
struct SamplingInfo{
  int sampling_index_;
  std::vector<std::vector<std::vector<Vec3>>>* area_lights_sampler_;
  float time = Sampling::_generate_random_float(0, 1);
};

struct Ray {
  Vec3 origin;
  Vec3 direction;
  SamplingInfo sampling_info;

  Interval interval_ = Interval(0.004f, INFINITY);

  Ray() = default;
  Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d) {}

  Vec3 at(float t) const { return origin + t * direction; }
};
} // namespace hasmet