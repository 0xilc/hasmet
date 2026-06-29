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
  
  float pi = glm::pi<float>();
  Vec3 T = {2.0f * pi * local_p.z, 0, -2.0f * pi * local_p.x};
  Vec3 B = {pi * local_p.y * std::cos(phi), -pi * std::sin(theta), pi * local_p.y * std::sin(phi)};
  
  rec.tangents[0] = glm::normalize(T);
  rec.tangents[1] = glm::normalize(B);
  
  rec.uv = Vec2(u, v);
  return true;
}

AABB Sphere::get_aabb() const { return local_aabb_; }

float Sphere::get_area() const {
  return 4.0f * glm::pi<float>() * radius_ * radius_;
}

SurfaceSample Sphere::sample_surface(const Vec2& u) const {
  // Uniform sampling on sphere surface
  float z = 1.0f - 2.0f * u.x;
  float r = std::sqrt(std::max(0.0f, 1.0f - z * z));
  float phi = 2.0f * glm::pi<float>() * u.y;

  Vec3 local_n(r * std::cos(phi), r * std::sin(phi), z);
  Vec3 p = center_ + radius_ * local_n;

  SurfaceSample ss;
  ss.p = p;
  ss.n = local_n;
  ss.pdf = 1.0f / get_area();
  return ss;
}

} // namespace hasmet