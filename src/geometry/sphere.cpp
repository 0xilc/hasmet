// src/geometry/sphere.cpp

#include "sphere.h"

#include <cmath>
#include "core/types.h"

namespace hasmet {
Sphere::Sphere(const Vec3& center, float radius)
    : center_(center), radius_(radius) {
  local_aabb_ = AABB(center - Vec3(radius, radius, radius),
               center + Vec3(radius, radius, radius));
}

bool Sphere::intersect(Ray& r, HitRecord& rec) const {
  Vec3 oc = r.origin - center_;

  double a = glm::dot(r.direction, r.direction);
  double half_b = glm::dot(oc, r.direction);
  double c = glm::dot(oc, oc) - radius_ * radius_;

  double discriminant = half_b * half_b - a * c;

  if (discriminant < 0) {
    return false;
  }

  double sqrtd = sqrt(discriminant);

  double root = (-half_b - sqrtd) / a;
  if (root < r.t_min || root > r.t_max) {
    root = (-half_b + sqrtd) / a;
    if (root < r.t_min || root > r.t_max) {
      return false;
    }
  }

  rec.t = static_cast<float>(root);
  rec.p = r.at(rec.t);
  Vec3 outward_normal = (rec.p - center_) / radius_;
  rec.set_face_normal(r, outward_normal);
  return true;
}

AABB Sphere::get_aabb() const { return local_aabb_; }
} // namespace hasmet