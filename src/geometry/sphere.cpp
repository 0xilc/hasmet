// src/geometry/sphere.cpp

#include "sphere.h"

#include <cmath>
#include "core/types.h"
#include <glm/gtc/constants.hpp>

namespace hasmet {
Sphere::Sphere(const Vec3& center, float radius)
    : center_(center), radius_(radius) {
  local_aabb_ = AABB(center - Vec3(radius, radius, radius),
               center + Vec3(radius, radius, radius));
}

bool Sphere::intersect(Ray& r, HitRecord& rec) const {
  Vec3 oc = r.origin - center_;

  float a = glm::dot(r.direction, r.direction);
  float half_b = glm::dot(oc, r.direction);
  float c = glm::dot(oc, oc) - radius_ * radius_;

  float discriminant = half_b * half_b - a * c;

  if (discriminant < 0) {
    return false;
  }

  float sqrtd = sqrt(discriminant);

  float root = (-half_b - sqrtd) / a;
  if (root < r.t_min || root > r.t_max) {
    root = (-half_b + sqrtd) / a;
    if (root < r.t_min || root > r.t_max) {
      return false;
    }
  }

  rec.t = root;
  rec.p = r.at(rec.t);
  Vec3 outward_normal = (rec.p - center_) / radius_;
  rec.set_face_normal(r, outward_normal);

  Vec3 local_p = (rec.p - center_) / radius_;
  float phi = std::atan2(local_p.z, local_p.x);
  float theta = std::acos(local_p.y);

  float u = (-phi + glm::pi<float>()) / (2.0f * glm::pi<float>());
  float v = theta / glm::pi<float>();

  Vec3 t_vec = Vec3(-local_p.z, 0.0f, local_p.x);
  rec.tangent = glm::normalize(t_vec);
  
  rec.uv = Vec2(u, v);
  return true;
}

AABB Sphere::get_aabb() const { return local_aabb_; }
} // namespace hasmet