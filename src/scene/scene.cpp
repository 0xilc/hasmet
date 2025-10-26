#include "scene.h"

Scene::Scene() {}

void Scene::add_shape(std::unique_ptr<Shape> shape) {
  shapes_.push_back(std::move(shape));
}

Material* Scene::add_material(std::unique_ptr<Material> material) {
  materials_.push_back(std::move(material));
  return materials_.back().get();
}

void Scene::add_point_light(std::unique_ptr<PointLight> light){
  point_lights_.push_back(std::move(light));
}

void Scene::add_ambient_light(std::unique_ptr<AmbientLight> light) {
  ambient_light_ = std::move(light);
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

  return hit_anything;
}