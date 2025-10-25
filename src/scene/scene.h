#pragma once

#include <memory>
#include <vector>

#include "geometry/shape.h"

class Scene {
 public:
  Scene();

  void add(std::unique_ptr<Shape> shape);

  bool intersect(const Ray& r, HitRecord& rec) const;

 private:
  std::vector<std::unique_ptr<Shape>> shapes_;
};