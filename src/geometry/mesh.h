#pragma once
#include <glm/glm.hpp>

#include "accelerator/bvh.h"
#include "accelerator/hittable.h"
#include "geometry/triangle.h"
#include "core/types.h"

namespace hasmet {
class Mesh : public Hittable {
 public:
  Mesh(std::vector<std::shared_ptr<Triangle>>& faces, int material_id);
  Mesh(const std::shared_ptr<BVH>& blas, int material_id);

  virtual bool intersect(Ray& ray, HitRecord& rec) const override;
  virtual AABB get_aabb() const override;

  std::shared_ptr<BVH> blas_;

 private:
  AABB local_aabb_;
  int material_id_;
};
} // namespace hasmet