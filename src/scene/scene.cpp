#include "scene.h"

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
  return bvh_root_->intersect(r, rec);
}

void Scene::build_bvh() {
  bvh_root_ = std::make_unique<BvhNode>(objects_.begin(), objects_.end());
}