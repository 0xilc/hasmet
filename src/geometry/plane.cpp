#include "plane.h"
#include "core/types.h"

namespace hasmet {
Plane::Plane(const Vec3& center, const Vec3& normal)
    : center_(center), normal_(normal) {
  local_aabb_ = AABB(Vec3(-INFINITY), Vec3(INFINITY));
}

bool Plane::intersect(Ray& ray, HitRecord& rec) const {
  float denom = glm::dot(normal_, ray.direction);
  if (std::abs(denom) > 1e-6f) {
    Vec3 p0l0 = center_ - ray.origin;
    float t = glm::dot(p0l0, normal_) / denom;
    if (ray.t_min <= t && ray.t_max >= t) {
      rec.t = t;
      rec.p = ray.origin + t * ray.direction;
      rec.normal = glm::normalize(normal_);
      return true;
    }
  }

  return false;
}

AABB Plane::get_aabb() const { return local_aabb_; }
} // namespace hasmet