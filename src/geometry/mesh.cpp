#include "mesh.h"

#include <glm/gtc/matrix_transform.hpp>
#include "core/types.h"

namespace hasmet {
Mesh::Mesh(std::vector<Triangle>& faces) {
  faces_ = faces;
  blas_ = BVH<Triangle>();
  blas_.build(faces_);
  local_aabb_ = blas_.get_root_aabb();
}

bool Mesh::intersect(Ray& ray, HitRecord& rec) const {
  return (blas_.intersect(ray, rec));
}

AABB Mesh::get_aabb() const { return local_aabb_; }
} // namespace hasmet