#pragma once
#include "light.h"

struct AmbientLight : public Light {
  Color radiance;

  AmbientLight(const Color& rad) : radiance(rad) {}
};