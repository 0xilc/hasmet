#pragma once

#include <memory>
#include <vector>

#include "light/ambient_light.h"
#include "light/point_light.h"
#include "material/material.h"
#include "accelerator/bvh.h"

class Scene {
 public:
  Scene();

  bool intersect(Ray& r, HitRecord& rec) const;
  
  void add_shape(std::unique_ptr<Hittable> shape);
  void add_point_light(std::unique_ptr<PointLight> light);
  void add_ambient_light(std::unique_ptr<AmbientLight> light);
  void build_bvh();

  std::unique_ptr<BvhNode> bvh_root_;
  std::vector<std::shared_ptr<Hittable>> objects_;
  std::vector<std::unique_ptr<Material>> materials_;
  std::vector<std::unique_ptr<PointLight>> point_lights_;
  std::unique_ptr<AmbientLight> ambient_light_;
};