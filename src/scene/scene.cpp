#include "scene.h"
#include "core/logging.h"
Scene::Scene() {}

void Scene::add_shape(std::unique_ptr<Hittable> shape) {
  objects_.push_back(std::move(shape));
}

void Scene::add_point_light(std::unique_ptr<PointLight> light) {
  point_lights_.push_back(std::move(light));
}

void Scene::add_ambient_light(std::unique_ptr<AmbientLight> light) {
  ambient_light_ = std::move(light);
}

bool Scene::intersect(Ray& r, HitRecord& rec) const {
  bool hit = false;
  float closest = INFINITY;

  if(bvh_root_->intersect(r, rec)) {
    hit = true;
    closest = rec.t;  
  }

  for (const std::shared_ptr<Plane>& plane : planes_) {
    HitRecord temp_rec;
    Ray temp_ray = r;
    if (plane->intersect(temp_ray, temp_rec)) {
      hit = true;
      if (temp_rec.t < closest) {
        closest = temp_rec.t;
        rec = temp_rec;
        r = temp_ray;
      }
    }
  }

  return hit;
}

void Scene::build_bvh() {
  bvh_root_ = std::make_unique<BvhNode>(objects_.begin(), objects_.end());
}