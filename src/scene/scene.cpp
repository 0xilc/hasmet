#include "scene.h"

Scene::Scene() {}

void Scene::add_shape(std::unique_ptr<Hittable> shape) {
  objects_.push_back(std::move(shape));
}

void Scene::add_point_light(std::unique_ptr<PointLight> light){
  point_lights_.push_back(std::move(light));
}

void Scene::add_ambient_light(std::unique_ptr<AmbientLight> light) {
  ambient_light_ = std::move(light);
}

bool Scene::intersect(Ray& r, HitRecord& rec) const {
 return bvh_root_->intersect(r, rec);
  //HitRecord temp_rec;
  //bool hit_anything = false;
  //auto closest_so_far = r.interval_.max;
  //for (const auto& shape : objects_) {
  //  Ray temp_ray = r;
  //  temp_ray.interval_.max = closest_so_far;

  //  if (shape->intersect(temp_ray, temp_rec)){
  //    hit_anything = true;
  //    closest_so_far = temp_rec.t;
  //    rec = temp_rec;
  //  }
  //}

  //return hit_anything;
}

void Scene::build_bvh() { bvh_root_ = std::make_unique<BvhNode>(objects_, 0, objects_.size() -1); }