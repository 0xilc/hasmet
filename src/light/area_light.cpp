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

float AreaLight::pdf_li(const HitRecord& rec, const Vec3& wi) const {
  // Intersect ray with the light plane to find the hit point
  float denom = glm::dot(normal, wi);
  if (std::abs(denom) < 1e-8f) return 0.0f;

  float t = glm::dot(position - rec.p, normal) / denom;
  if (t <= 0) return 0.0f;

  Vec3 hit_p = rec.p + wi * t;
  Vec3 offset = hit_p - position;
  float u_coord = glm::dot(offset, u);
  float v_coord = glm::dot(offset, v);
  float half = size * 0.5f;
  if (std::abs(u_coord) > half || std::abs(v_coord) > half) return 0.0f;

  float area = size * size;
  float dist2 = t * t;
  float cos_light = std::abs(denom);

  return dist2 / (cos_light * area);
}

} // namespace hasmet
