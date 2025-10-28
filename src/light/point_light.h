#pragma once
#include "light.h"

struct PointLight : public Light {
  glm::vec3 position;
  Color intensity;

  PointLight(const glm::vec3& pos, const Color& i)
      : position(pos), intensity(i) {}
};