#include "scene.h"

#include "core/logging.h"
#include <memory>
namespace hasmet {
Scene::Scene() {}

void Scene::add_shape(Instance shape) {
  objects_.push_back(std::move(shape));
}

void Scene::add_point_light(std::unique_ptr<PointLight> light) {
  point_lights_.push_back(std::move(light));
}

void Scene::add_ambient_light(std::unique_ptr<AmbientLight> light) {
  ambient_light_ = std::move(light);
}

void Scene::add_material(std::unique_ptr<Material> mat) {
  materials_.push_back(std::move(mat));
}

const Material* Scene::get_material(int id) const {
  if (id < 0 || id >= materials_.size()) {
    LOG_ERROR("Invalid material id: " << id);
  }

  return materials_[id].get();
}

bool Scene::intersect(Ray& r, HitRecord& rec) const {
  bool hit = false;

  if (bvh_.intersect(r, rec)) {
    hit = true;
  }

  for (const auto& plane : planes_) {
    HitRecord temp_rec;
    if (plane.intersect(r, temp_rec)) {
      if (!hit || temp_rec.t < rec.t) {
        hit = true;
        rec = temp_rec;
        r.t_max = rec.t;
      }
    }
  }

  return hit;
}

bool Scene::is_occluded(const Ray& r) const {
  if (bvh_.is_occluded(r)) return true;

  HitRecord temp_rec;
  Ray shadow_ray = r;
  for (const auto& plane : planes_) {
    if (plane.intersect(shadow_ray, temp_rec)) return true;
  }

  return false;
}

void Scene::build_bvh() { bvh_.build(objects_); }

int Scene::get_total_light_count() const {
  return static_cast<int>(point_lights_.size() + area_lights_.size() +
                          spot_lights_.size() + light_indices_.size());
}

float Scene::light_pdf(const Ray& ray, const HitRecord& rec) const {
  int total = get_total_light_count();
  if (total == 0) return 0.0f;

  float light_pick_pdf = 1.0f / total;

  // Check if the hit is an object light
  if (!rec.radiance.has_value()) return 0.0f;

  // Find which object light was hit and get its pdf
  for (int idx : light_indices_) {
    float pdf = objects_[idx].pdf_li(rec, ray.direction);
    if (pdf > 0.0f) {
      return pdf * light_pick_pdf;
    }
  }

  // Check area lights
  for (const auto& al : area_lights_) {
    float pdf = al->pdf_li(rec, ray.direction);
    if (pdf > 0.0f) {
      return pdf * light_pick_pdf;
    }
  }

  return 0.0f;
}

} // namespace hasmet