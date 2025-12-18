#include "mesh.h"

#include <glm/gtc/matrix_transform.hpp>
#include "core/types.h"

namespace hasmet {
Mesh::Mesh(std::vector<std::shared_ptr<Triangle>>& faces, int material_id,
           Vec3 motion_blur = Vec3(0))
    : material_id_(material_id) {
  motion_blur_ = motion_blur;
  has_motion_blur_ = true;
  std::vector<std::shared_ptr<Hittable>> hittable_faces;
  hittable_faces.reserve(faces.size());

  for (const auto& face : faces) {
    hittable_faces.push_back(face);
  }
  blas_ = std::make_shared<BVH>();
  blas_->build(hittable_faces);
  aabb_ = blas_->get_root_aabb();
  aabb_.apply_transformation(this->transform_);

  AABB tmp = aabb_;

  glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), motion_blur_);

  tmp.apply_transformation(translation_matrix);
  aabb_.expand(tmp);
}

bool Mesh::local_intersect(Ray& ray, HitRecord& rec) const {
  if (blas_->intersect(ray, rec)) {
    rec.material_id = material_id_;
    return true;
  }
  return false;
}

Mesh::Mesh(const std::shared_ptr<BVH>& blas, int material_id,
           Vec3 motion_blur = Vec3(0))
    : blas_(blas), material_id_(material_id) {
  motion_blur_ = motion_blur;
  aabb_ = blas_->get_root_aabb();

  AABB tmp = aabb_;

  glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), motion_blur_);

  tmp.apply_transformation(translation_matrix);
  aabb_.expand(tmp);
}

AABB Mesh::get_aabb() const { return aabb_; }
} // namespace hasmet