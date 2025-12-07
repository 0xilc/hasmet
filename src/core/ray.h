#pragma once

#include <glm/glm.hpp>
#include <limits>
#include "interval.h"
#include "core/sampling.h"

struct SamplingInfo{
  int sampling_index_;
  std::vector<std::vector<std::vector<glm::vec3>>>* area_lights_sampler_;
  float time = Sampling::_generate_random_float(0, 1);
};

struct Ray {
  glm::vec3 origin;
  glm::vec3 direction;
  SamplingInfo sampling_info;

  Interval interval_ = Interval(0.004f, INFINITY);

  Ray() = default;
  Ray(const glm::vec3& o, const glm::vec3& d) : origin(o), direction(d) {}

  glm::vec3 at(float t) const { return origin + t * direction; }
};