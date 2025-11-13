#include "mesh.h"

Mesh::Mesh(std::vector<std::shared_ptr<Triangle>>& faces, int material_id)
    : material_id_(material_id) {
  blas_ = std::make_unique<BvhNode>(faces.begin(), faces.end());
  aabb_ = blas_->getAABB();
}

bool Mesh::intersect(Ray& ray, HitRecord& rec) const {
  return blas_->intersect(ray, rec);
}

AABB Mesh::getAABB() const { return aabb_; }