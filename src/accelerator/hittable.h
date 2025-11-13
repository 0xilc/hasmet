#pragma once

#include "core/aabb.h"
#include "core/hit_record.h"
#include "core/interval.h"
#include "core/ray.h"

class Hittable {
 public:
  virtual ~Hittable() = default;

  virtual bool intersect(Ray& ray, HitRecord& rec) const = 0;
  virtual AABB getAABB() const = 0;

  void setTransform(const glm::mat4& m) {
    transform_ = m;
    inverse_transform_ = glm::inverse(m);
    inverse_transpose_transform_ = glm::transpose(inverse_transform_);
  }

 protected:
  glm::mat4 transform_{1.0f};
  glm::mat4 inverse_transform_{1.0f};
  glm::mat4 inverse_transpose_transform_{1.0f};
};
