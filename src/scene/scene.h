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

  void add(std::unique_ptr<Shape> shape);

  bool intersect(const Ray& r, HitRecord& rec) const;

 private:
  std::vector<std::unique_ptr<Shape>> shapes_;
  std::vector<std::unique_ptr<Material>> materials_;
  std::unique_ptr<AmbientLight> ambient_light_;
  std::vector<std::unique_ptr<PointLight>> point_light_;
};