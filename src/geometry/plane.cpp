#include "plane.h"
#include "core/types.h"

namespace hasmet {
Plane::Plane(const Vec3& center, const Vec3& normal, int material_id)
    : center_(center), normal_(normal), material_id_(material_id) {
  aabb_ = AABB(Vec3(-INFINITY), Vec3(INFINITY));
  aabb_.apply_transformation(transform_);
}

bool Plane::local_intersect(Ray& ray, HitRecord& rec) const {
  float denom = glm::dot(normal_, ray.direction);
  if (std::abs(denom) > 1e-6f) {
    Vec3 p0l0 = center_ - ray.origin;
    float t = glm::dot(p0l0, normal_) / denom;
    if (ray.t_min <= t && ray.t_max >= t) {
      rec.t = t;
      rec.p = ray.origin + t * ray.direction;
      rec.normal = glm::normalize(normal_);
      rec.material_id = material_id_;
      return true;
    }
  }

  return false;
}

AABB Plane::get_aabb() const { return aabb_; }
} // namespace hasmet