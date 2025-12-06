#pragma once
#include "core/color.h"
#include "light.h"

struct AreaLight : public Light {
  glm::vec3 position;
  glm::vec3 normal;
  float size;
  Color radiance;

  AreaLight(const glm::vec3& pos, const glm::vec3& normal, float size,
            const Color& rad)
      : position(pos), normal(normal), size(size), radiance(rad) {}
};