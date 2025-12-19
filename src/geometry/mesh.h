#pragma once
#include <glm/glm.hpp>

#include "accelerator/bvh.h"
#include "accelerator/hittable.h"
#include "geometry/triangle.h"
#include "core/types.h"

namespace hasmet {
class Mesh : public Hittable {
 public:
  Mesh(std::vector<Triangle>& faces);

  virtual bool intersect(Ray& ray, HitRecord& rec) const override;
  virtual AABB get_aabb() const override;

  BVH<Triangle> blas_;
  std::vector<Triangle> faces_;
 private:
  AABB local_aabb_;
};
} // namespace hasmet