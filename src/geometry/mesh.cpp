#include "mesh.h"

Mesh::Mesh(std::vector<std::shared_ptr<Triangle>>& faces, int material_id)
    : material_id_(material_id) {
  blas_ = std::make_shared<BvhNode>(faces.begin(), faces.end());
  aabb_ = blas_->getAABB();
  aabb_.apply_transformation(this->transform_);
}

bool Mesh::local_intersect(Ray& ray, HitRecord& rec) const {

  if(blas_->intersect(ray, rec)){
    rec.material_id = material_id_;
    return true;
  }
  return false;
}

Mesh::Mesh(const std::shared_ptr<BvhNode>& blas, int material_id)
    : blas_(blas), material_id_(material_id) {
  aabb_ = blas_->getAABB();
}

AABB Mesh::getAABB() const { return aabb_; }