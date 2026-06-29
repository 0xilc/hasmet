#pragma once
#include "hittable.h"
#include "light/light.h"
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "core/sampler.h"

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
    rec.texture_ids = &this->texture_ids_;
    if (this->is_light()) {
      rec.radiance = this->radiance_;
    }
    if (has_motion_blur_) rec.p += motion_blur_ * ray.time;
    
    glm::mat normal_matrix = glm::transpose(glm::mat3(inv_transform_));
    rec.normal = glm::normalize(normal_matrix * rec.normal);
    
    // glm::mat3 model_rot_scale = glm::mat3(glm::inverse(transform_));
    // if(rec.tangents){
    //   rec.tangents[0] = glm::normalize(model_rot_scale * rec.tangents[0]);
    //   rec.tangents[1] = glm::normalize(model_rot_scale * rec.tangents[1]);
    // }

    ray.t_max = local_ray.t_max;
    return true;
  }

  virtual AABB get_aabb() const override { return world_aabb_; }

  bool is_light() const { return glm::length(radiance_) > 0; }

  LightSample sample_li(const HitRecord& rec, const Vec2& u) const {
    SurfaceSample ss = object_->sample_surface(u);
    // Transform sampled point and normal to world space
    Vec3 world_p = Vec3(transform_ * glm::vec4(ss.p, 1.0f));
    glm::mat3 normal_matrix = glm::transpose(glm::mat3(inv_transform_));
    Vec3 world_n = glm::normalize(normal_matrix * ss.n);

    Vec3 wi_full = world_p - rec.p;
    float dist2 = glm::dot(wi_full, wi_full);
    float dist = std::sqrt(dist2);
    Vec3 wi = wi_full / dist;

    float cos_light = std::abs(glm::dot(world_n, -wi));
    if (cos_light < 1e-8f) return {};

    // Convert area PDF to solid-angle PDF: pdf_area * dist^2 / cos_light
    // Account for transform scaling on area
    float scale_factor = glm::length(glm::cross(
        Vec3(transform_[0]), Vec3(transform_[1])));
    float world_area = object_->get_area() * scale_factor;
    float pdf_area = 1.0f / world_area;
    float pdf_solid = pdf_area * dist2 / cos_light;

    Color L = radiance_ * cos_light / dist2;
    return { L, wi, pdf_solid, dist };
  }

  float pdf_li(const HitRecord& rec, const Vec3& wi) const {
    // Cast a ray to find intersection with this object
    Ray test_ray(rec.p + wi * 1e-4f, wi);
    HitRecord test_rec;

    Ray local_ray = test_ray;
    local_ray.origin = Vec3(inv_transform_ * glm::vec4(local_ray.origin, 1.0f));
    local_ray.direction = Vec3(inv_transform_ * glm::vec4(local_ray.direction, 0.0f));

    if (!object_->intersect(local_ray, test_rec)) return 0.0f;

    float dist2 = test_rec.t * test_rec.t;
    glm::mat3 normal_matrix = glm::transpose(glm::mat3(inv_transform_));
    Vec3 world_n = glm::normalize(normal_matrix * test_rec.normal);
    float cos_light = std::abs(glm::dot(world_n, -wi));
    if (cos_light < 1e-8f) return 0.0f;

    float scale_factor = glm::length(glm::cross(
        Vec3(transform_[0]), Vec3(transform_[1])));
    float world_area = object_->get_area() * scale_factor;
    float pdf_area = 1.0f / world_area;
    return pdf_area * dist2 / cos_light;
  }

  Color radiance_{0.0f};
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