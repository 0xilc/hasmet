// src/geometry/sphere.cpp

#include "sphere.h"

#include <cmath>

Sphere::Sphere(const glm::vec3& center, float radius, int material_id)
    : center_(center), radius_(radius), material_id_(material_id) {}

bool Sphere::intersect(Ray& r, HitRecord& rec) const {
  glm::vec3 oc = r.origin - center_;

  double a = glm::dot(r.direction, r.direction);
  double half_b = glm::dot(oc, r.direction);
  double c = glm::dot(oc, oc) - radius_ * radius_;

  double discriminant = half_b * half_b - a * c;

  if (discriminant < 0) {
    return false;
  }

  double sqrtd = sqrt(discriminant);

  double root = (-half_b - sqrtd) / a;
  if (root < r.interval_.min || root > r.interval_.max) {
    root = (-half_b + sqrtd) / a;
    if (root < r.interval_.min || root > r.interval_.max) {
      return false;
    }
  }

  rec.t = static_cast<float>(root);
  rec.p = r.at(rec.t);
  glm::vec3 outward_normal = (rec.p - center_) / radius_;
  rec.set_face_normal(r, outward_normal);
  rec.material_id = material_id_;
  return true;
}

AABB Sphere::getAABB() const { return aabb_; }