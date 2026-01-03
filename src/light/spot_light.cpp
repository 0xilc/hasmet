#include "spot_light.h"
#include "glm/exponential.hpp"

namespace hasmet {

LightSample SpotLight::sample_li(const HitRecord& rec, const Vec2 &u) const {
  Vec3 wi_full = position - rec.p;
  float dist2 = glm::dot(wi_full, wi_full);
  float dist = glm::sqrt(dist2);
  Vec3 wi = wi_full / dist;

  float cos_alpha = glm::dot(this->direction, -wi);

  float cos_half_coverage = glm::cos(glm::radians(coverage_angle) * 0.5f);
  float cos_half_falloff = glm::cos(glm::radians(falloff_angle) * 0.5f);

  float s = 0.0f;
  if (cos_alpha >= cos_half_falloff) {
    s = 1.0f;
  }
  else if (cos_alpha <= cos_half_coverage) {
    s = 0.0f;
  }
  else {
    float num = cos_alpha - cos_half_coverage;
    float den = cos_half_falloff - cos_half_coverage;
    s = glm::pow(num / den, 4.0f);
  }

  // Radiance
  Color L = (s > 0.0f) ? (Color(intensity) * s / dist2) : Color(0.0f);
  return {L, wi, 1.0f, dist };
}

} // namespace hasmet