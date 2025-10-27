#pragma once

#include <memory>
#include <vector>

#include "geometry/shape.h"
#include "light/ambient_light.h"
#include "light/point_light.h"
#include "material/material.h"

class Scene {
 public:
  Scene();

  void add_shape(std::unique_ptr<Shape> shape);
  Material* add_material(std::unique_ptr<Material> material);
  void add_point_light(std::unique_ptr<PointLight> light);
  void add_ambient_light(std::unique_ptr<AmbientLight> light);
  bool intersect(const Ray& r, HitRecord& rec) const;

  std::vector<std::unique_ptr<Shape>> shapes_;
  std::vector<std::unique_ptr<Material>> materials_;
  std::unique_ptr<AmbientLight> ambient_light_;
  std::vector<std::unique_ptr<PointLight>> point_lights_;
};