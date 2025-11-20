#include "mesh.h"

Mesh::Mesh(std::vector<std::shared_ptr<Triangle>>& faces, int material_id)
    : material_id_(material_id) {
  std::vector<std::shared_ptr<Hittable>> hittable_faces;
  hittable_faces.reserve(faces.size());

  for (const auto& face : faces) {
    hittable_faces.push_back(face);
  }
  blas_ = std::make_shared<BVH>();
  blas_->build(hittable_faces);
  aabb_ = blas_->get_root_aabb();
  aabb_.apply_transformation(this->transform_);
}

bool Mesh::local_intersect(Ray& ray, HitRecord& rec) const {
    return blas_->intersect(ray, rec);
}

Mesh::Mesh(const std::shared_ptr<BVH>& blas, int material_id)
    : blas_(blas), material_id_(material_id) {
  aabb_ = blas_->get_root_aabb();
}

AABB Mesh::get_aabb() const { return aabb_; }