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

const Material* Scene::get_material(int id) {
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

} // namespace hasmet