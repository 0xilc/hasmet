#pragma once

#include "core/aabb.h"
#include "core/hit_record.h"
#include "core/interval.h"
#include "core/ray.h"

class Hittable {
 public:
  virtual ~Hittable() = default;

  bool intersect(Ray& ray, HitRecord& rec) const;
 
  virtual AABB getAABB() const = 0;

  void set_transform(const glm::mat4& m);

 protected:
  virtual bool local_intersect(Ray& ray, HitRecord& rec) const = 0;
  glm::mat4 transform_{1.0f};
  glm::mat4 inverse_transform_{1.0f};
  glm::mat4 inverse_transpose_transform_{1.0f};
  AABB aabb_;
};
