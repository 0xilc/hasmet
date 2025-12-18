#pragma once

#include "core/aabb.h"
#include "core/hit_record.h"
#include "core/interval.h"
#include "core/ray.h"
#include "core/types.h"

namespace hasmet{
class Hittable {
 public:
  virtual ~Hittable() = default;

  bool intersect(Ray& ray, HitRecord& rec) const;
 
  void set_transform(const glm::mat4& m);
  virtual AABB get_aabb() const = 0;
  glm::mat4 get_transform() const { return transform_; }

 protected:
  virtual bool local_intersect(Ray& ray, HitRecord& rec) const = 0;
  glm::mat4 transform_{1.0f};
  glm::mat4 inverse_transform_{1.0f};
  glm::mat4 inverse_transpose_transform_{1.0f};
  Vec3 motion_blur_;
  AABB aabb_;
  bool is_identity_transform_ = true;
  bool has_motion_blur_ = false;
};
} // namespace hasmet