#pragma once

#include <algorithm>
#include <memory>
#include <random>
#include <vector>

#include "hittable.h"

class BvhNode : public Hittable {
 public:
  BvhNode(std::vector<std::shared_ptr<Hittable>>& objects, int begin, int end);

  bool intersect(Ray& ray, HitRecord& rec) const override;
  AABB getAABB() const override;

 private:
  std::shared_ptr<Hittable> left_;
  std::shared_ptr<Hittable> right_;
  AABB bounding_box_;
};
