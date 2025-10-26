#pragma once
#include "light.h"

struct AmbientLight : public Light {
  Spectrum radiance;

  AmbientLight(const Spectrum& rad) : radiance(rad) {}
};