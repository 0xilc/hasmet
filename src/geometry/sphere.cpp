#include "sphere.h"

Sphere::Sphere(const Vec3& center, float radius)
    : center_(center), radius_(radius) {}

bool Sphere::intersect(const Ray& r, HitRecord& rec) const {
  Vec3 oc = r.origin - center_;
  float a = r.direction.length_squared();
  float half_b = dot(oc, r.direction);
  float c = oc.length_squared() - radius_ * radius_;

  float discriminant = half_b * half_b - a * c;
  if (discriminant < 0) {
    return false;
  }

  float sqrtd = sqrt(discriminant);
  float root = (-half_b - sqrtd) / a;
  if (root < r.tmin || root > r.tmax){
    root = (-half_b + sqrtd) / a;
    if (root < r.tmin || root > r.tmax) {
      return false;
    }
  }

  rec.t = root;
  rec.p = r.at(rec.t);
  Vec3 outward_normal = normalize(rec.p - center_);
  rec.set_face_normal(r, outward_normal);
}