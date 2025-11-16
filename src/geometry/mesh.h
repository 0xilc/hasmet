#pragma once
#include <glm/glm.hpp>

#include "accelerator/bvh.h"
#include "accelerator/hittable.h"
#include "geometry/triangle.h"

class Mesh : public Hittable {
 public:
  Mesh(std::vector<std::shared_ptr<Triangle>>& faces, int material_id);
  Mesh(std::shared_ptr<BvhNode> blas, int material_id);

  virtual bool local_intersect(Ray& ray, HitRecord& rec) const override;
  virtual AABB getAABB() const override;

  std::shared_ptr<BvhNode> blas_;
 private:
  int material_id_;
};
