#include "scene.h"

Scene::Scene() {}

void Scene::add(std::unique_ptr<Shape> shape) {
  shapes_.push_back(std::move(shape));
}

bool Scene::intersect(const Ray& r, HitRecord& rec) const {
  HitRecord temp_rec;
  bool hit_anything = false;
  auto closest_so_far = r.tmax;

  for (const auto& shape : shapes_) {
    Ray temp_ray = r;
    temp_ray.tmax = closest_so_far;

    if (shape->intersect(temp_ray, temp_rec)){
      hit_anything = true;
      closest_so_far = temp_rec.t;
      rec = temp_rec;
    }
  }
}