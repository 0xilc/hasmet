#pragma once
#include "light.h"
#include "core/color.h"

struct AmbientLight : public Light {
  Color radiance;

  AmbientLight(const Color& rad) : radiance(rad) {}
};