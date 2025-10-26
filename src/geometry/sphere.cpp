// src/geometry/sphere.cpp

#include "sphere.h"

#include <cmath>

Sphere::Sphere(const Vec3& center, float radius)
    : center_(center), radius_(radius) {}

bool Sphere::intersect(const Ray& r, HitRecord& rec) const {
  Vec3 oc = r.origin - center_;

  double a = r.direction.length_squared();
  double half_b = dot(oc, r.direction);
  double c = oc.length_squared() - radius_ * radius_;

  double discriminant = half_b * half_b - a * c;

  if (discriminant < 0) {
    return false;
  }

  double sqrtd = sqrt(discriminant);

  double root = (-half_b - sqrtd) / a;
  if (root < r.tmin || root > r.tmax) {
    root = (-half_b + sqrtd) / a;
    if (root < r.tmin || root > r.tmax) {
      return false;
    }
  }

  rec.t = static_cast<float>(root);
  rec.p = r.at(rec.t);
  Vec3 outward_normal = (rec.p - center_) / radius_;
  rec.set_face_normal(r, outward_normal);

  return true;
}