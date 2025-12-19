#include "mesh.h"

#include <glm/gtc/matrix_transform.hpp>
#include "core/types.h"

namespace hasmet {
Mesh::Mesh(std::vector<std::shared_ptr<Triangle>>& faces) {
  std::vector<std::shared_ptr<Hittable>> hittable_faces;
  hittable_faces.reserve(faces.size());

  for (const auto& face : faces) {
    hittable_faces.push_back(face);
  }
  
  blas_ = std::make_shared<BVH>();
  blas_->build(hittable_faces);
  local_aabb_ = blas_->get_root_aabb();
}

bool Mesh::intersect(Ray& ray, HitRecord& rec) const {
  return (blas_->intersect(ray, rec));
}

Mesh::Mesh(const std::shared_ptr<BVH>& blas)
    : blas_(blas){
  local_aabb_ = blas_->get_root_aabb();  
}

AABB Mesh::get_aabb() const { return local_aabb_; }
} // namespace hasmet