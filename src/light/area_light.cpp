#include "area_light.h"

namespace hasmet {

LightSample AreaLight::sample_li(const HitRecord& rec, const Vec2 &u) const {
  Vec3 sampled_light_point =
      position + (this->u * (u.x - 0.5f) + (this->v * (u.y - 0.5f))) * size;

  Vec3 wi_full = sampled_light_point - rec.p;
  float dist2 = glm::dot(wi_full, wi_full);
  float dist = glm::sqrt(dist2);
  Vec3 wi = wi_full / dist;

  float cos_light = glm::abs(glm::dot(this->normal, -wi));

  float area = size * size;
  float pdf = 1.0f / area;

  Color L = Color(this->radiance) * (cos_light / dist2);

  return { L, wi, pdf, dist }; 
}
} // namespace hasmet
