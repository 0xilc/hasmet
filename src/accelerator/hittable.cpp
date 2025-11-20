#include "hittable.h"

bool Hittable::intersect(Ray& ray, HitRecord& rec) const {
  if (is_identity_transform_) {
    return local_intersect(ray, rec);
  }

  Ray local_ray;
  local_ray.origin = inverse_transform_ * glm::vec4(ray.origin, 1.0f);
  local_ray.direction = inverse_transform_ * glm::vec4(ray.direction, 0.0f);

  local_ray.interval_.max = ray.interval_.max;

  if (!local_intersect(local_ray, rec)) {
    return false;
  }
  rec.p = transform_ * glm::vec4(rec.p, 1.0f);
  rec.normal = glm::normalize(
      glm::vec3(inverse_transpose_transform_ * glm::vec4(rec.normal, 0.0f)));

  ray.interval_.max = local_ray.interval_.max;
  return true;
}

void Hittable::set_transform(const glm::mat4& m) {
  transform_ = m;
  inverse_transform_ = glm::inverse(m);
  inverse_transpose_transform_ = glm::transpose(inverse_transform_);
  aabb_.apply_transformation(m);
  is_identity_transform_ = false;
}