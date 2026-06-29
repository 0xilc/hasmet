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

float Mesh::get_area() const {
  float total = 0.0f;
  for (const auto& tri : faces_) {
    total += tri.get_area();
  }
  return total;
}

SurfaceSample Mesh::sample_surface(const Vec2& u) const {
  // Uniform triangle selection (not area-weighted — acceptable approximation)
  int n = faces_.size();
  int idx = std::min((int)(u.x * n), n - 1);
  float u_remapped = u.x * n - idx;

  SurfaceSample ss = faces_[idx].sample_surface(Vec2(u_remapped, u.y));
  // Adjust pdf: we picked 1 of n triangles uniformly, then sampled that triangle
  // P(point) = (1/n) * (1/tri_area) = 1 / (n * tri_area)
  ss.pdf = 1.0f / (n * faces_[idx].get_area());
  return ss;
}

} // namespace hasmet