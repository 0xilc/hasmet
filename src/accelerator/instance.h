#pragma once
#include "hittable.h"
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace hasmet {
class Instance : public Hittable {
 public:
  Instance(std::shared_ptr<Hittable> object) : object_(object), material_id_(1) {}

  // TODO: Solve temporal coupling between set_transform and set_motion_blur
  void set_transform(const glm::mat4& m) {
    transform_ = m;
    inv_transform_ = glm::inverse(m);
    world_aabb_ = object_->get_aabb();
    world_aabb_.apply_transformation(transform_);
  }

  void set_motion_blur(const Vec3& v) {
    if (glm::dot(v, v) > 1e-8f) {
      motion_blur_ = v;
      has_motion_blur_ = true;

      AABB end_aabb = world_aabb_;
      glm::mat4 motion_blur_m = glm::translate(glm::mat4(1.0f), v);
      end_aabb.apply_transformation(motion_blur_m);
      world_aabb_.expand(end_aabb);
    }
  }
  
  void set_material_id(int material_id) {
    material_id_ = material_id;
  };

  void set_texture_ids(const std::vector<int>& texture_ids) {
    texture_ids_ = texture_ids;
  }

  virtual bool intersect(Ray& ray, HitRecord& rec) const override {
    Ray local_ray = ray;
    if (has_motion_blur_) local_ray.origin -= motion_blur_ * ray.time;
    
    local_ray.origin = Vec3(inv_transform_ * glm::vec4(local_ray.origin, 1.0f));
    local_ray.direction = Vec3(inv_transform_ * glm::vec4(local_ray.direction, 0.0f));

    if (!object_->intersect(local_ray, rec)) return false;

    rec.p = ray.at(rec.t);
    rec.material_id = material_id_;
    
    if (has_motion_blur_) rec.p += motion_blur_ * ray.time;
    
    glm::mat3 normal_matrix = glm::transpose(glm::mat3(inv_transform_));
    rec.normal = glm::normalize(normal_matrix * rec.normal);
    
    ray.t_max = local_ray.t_max;
    return true;
  }

  virtual AABB get_aabb() const override { return world_aabb_; }

 private:
  std::shared_ptr<Hittable> object_;
  glm::mat4 transform_{1.0f};
  glm::mat4 inv_transform_{1.0f};
  AABB world_aabb_;
  Vec3 motion_blur_{0.0f};
  bool has_motion_blur_ = false;
  int material_id_;
  std::vector<int> texture_ids_;
};
}