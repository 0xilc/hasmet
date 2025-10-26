#pragma once
#include "light.h"

struct PointLight : public Light {
  glm::vec3 position;
  Spectrum intensity;

  PointLight(const glm::vec3& pos, const Spectrum& i)
      : position(pos), intensity(i) {}
};