#pragma once
#include <glm/glm.hpp>

#include "accelerator/bvh.h"
#include "accelerator/hittable.h"
#include "geometry/triangle.h"

class Mesh : public Hittable {
 public:
  Mesh(std::vector<std::shared_ptr<Triangle>>& faces, int material_id);

  virtual bool intersect(Ray& ray, HitRecord& rec) const override;
  virtual AABB getAABB() const override;

 private:
  std::unique_ptr<BvhNode> blas_;
  int material_id_;
  AABB aabb_;
};
