#include "environment_light.h"
#include "core/hit_record.h"
#include "glm/geometric.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "image/image_manager.h"
#include "light/light.h"
#include <limits>

namespace hasmet {
namespace {
Vec2 get_uv_from_dir(const Vec3 &d, const std::string &type) {
  float u, v;
  if (type == "latlong") {
    u = (1.0f + std::atan2(d.x, -d.z) / glm::pi<float>()) * 0.5f;
    v = glm::acos(glm::clamp(d.y, -1.0f, 1.0f)) / glm::pi<float>();
  } else {
    float r = (1.0f / glm::pi<float>()) *
              glm::acos(glm::clamp(-d.z, -1.0f, 1.0f)) /
              glm::sqrt(d.x * d.x + d.y * d.y + 1e-6f);
    float u = (r * d.x + 1.0f) * 0.5f;
    float v = (-r * d.y + 1.0f) * 0.5f;
  }
  return Vec2(u, v);
}

Vec3 sample_hemisphere_uniform(float r1, float r2) {
  float phi = 2.0f * glm::pi<float>() * r1;
  float cos_theta = 1.0f - r2;
  float sin_theta = glm::sqrt(1.0f - cos_theta * cos_theta);
  return Vec3(glm::cos(phi) * sin_theta, glm::sin(phi) * sin_theta, cos_theta);
}

Vec3 sample_hemisphere_cosine(float r1, float r2) {
  float phi = 2.0f * glm::pi<float>() * r1;
  float sin_theta = glm::sqrt(r2);
  float cos_theta = glm::sqrt(1.0f - r2);
  return Vec3(glm::cos(phi) * sin_theta, glm::sin(phi) * sin_theta, cos_theta);
}

inline Vec3 local_to_world(const Vec3 &local_v, const Vec3 &normal) {
  Vec3 w = glm::normalize(normal);
  Vec3 a = (glm::abs(w.x) > 0.9f) ? Vec3(0, 1, 0) : Vec3(1, 0, 0);

  Vec3 v = glm::normalize(glm::cross(w, a));
  Vec3 u = glm::cross(w, v);

  return u * local_v.x + v * local_v.y + w * local_v.z;
}
} // namespace

LightSample EnvironmentLight::sample_li(const HitRecord &rec,
                                        const Vec2 &u) const {
  Vec3 local_wi;
  float pdf;

  if (this->sampler == "cosine") {
    local_wi = sample_hemisphere_cosine(u.x, u.y);
    pdf = local_wi.z / glm::pi<float>();
  } else {
    local_wi = sample_hemisphere_uniform(u.x, u.y);
    pdf = 1.0f / (2.0f * glm::pi<float>());
  }

  Vec3 wi_world = local_to_world(local_wi, rec.normal);

  float u_tex, v_tex;
  Vec3 d = glm::normalize(wi_world);

  if (this->type == "latlong") {

    u_tex = (1.0f + std::atan2(d.x, -d.z) / glm::pi<float>()) * 0.5f;
    v_tex = std::acos(std::clamp(d.y, -1.0f, 1.0f)) / glm::pi<float>();
  } else {
    float r = (1.0f / glm::pi<float>()) *
              std::acos(std::clamp(-d.z, -1.0f, 1.0f)) /
              std::sqrt(d.x * d.x + d.y * d.y + 1e-6f);
    u_tex = (r * d.x + 1.0f) * 0.5f;
    v_tex = (-r * d.y + 1.0f) * 0.5f;
  }

  Color L = ImageManager::get_instance()->get(image_id).get_pixel(u_tex, v_tex);
  
  return { L, wi_world, pdf, std::numeric_limits<float>::infinity()};
}
} // namespace hasmet