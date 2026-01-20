#pragma once
#include "core/types.h"

namespace hasmet {
struct Medium {
  Color sigma_a;  // Absorption coefficient
  Color sigma_s;  // Scattering coefficient
  float g;        // Phase function asymmetry

  Medium(Color a = Color(0.0f), Color s = Color(0.0f), float g = 0.0f)
    : sigma_a(a), sigma_s(s), g(g) {}
  
  Color transmittance(float distance) const {
    Color sigma_t = sigma_a + sigma_s;
    return glm::exp(-sigma_t * distance);
  }
};
} // namespace hasmet