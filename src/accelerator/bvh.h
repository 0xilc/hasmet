#pragma once

#include <algorithm>
#include <iterator>
#include <memory>
#include <random>
#include <vector>

#include "core/aabb.h"
#include "core/ray.h"
#include "hittable.h"

class BvhNode : public Hittable {
 public:
  template <typename Iterator>
  BvhNode(Iterator begin, Iterator end);

  bool local_intersect(Ray& ray, HitRecord& rec) const override;

  AABB getAABB() const override;

 private:
  std::shared_ptr<Hittable> left_;
  std::shared_ptr<Hittable> right_;
  AABB bounding_box_;
};

template <typename Iterator>
BvhNode::BvhNode(Iterator begin, Iterator end) {
  std::vector<std::shared_ptr<Hittable>> objects(begin, end);

  size_t object_span = objects.size();
  AABB centroid_box;
  for (const auto& obj : objects) {
    centroid_box.expand(obj->getAABB().centroid());
  }
  int axis = centroid_box.longest_axis();
  //int axis = rand() % 3;

  if (object_span == 1) {
    left_ = right_ = objects[0];
  } else if (object_span == 2) {
    left_ = objects[0];
    right_ = objects[1];
  } else {
    size_t mid = object_span / 2;
    std::nth_element(objects.begin(), objects.begin() + mid, objects.end(),
                     [axis](const std::shared_ptr<Hittable>& a,
                            const std::shared_ptr<Hittable>& b) {
                       return a->getAABB().axis(axis).min <
                              b->getAABB().axis(axis).min;
                     });
    left_ = std::make_shared<BvhNode>(objects.begin(), objects.begin() + mid);
    right_ = std::make_shared<BvhNode>(objects.begin() + mid, objects.end());
  }

  bounding_box_ = AABB(left_->getAABB(), right_->getAABB());
}