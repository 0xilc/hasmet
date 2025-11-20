#pragma once

#include <vector>
#include <memory>
#include "core/aabb.h"
#include "core/ray.h"
#include "hittable.h"

struct BvhBuildNode;

struct LinearBvhNode {
  AABB bbox;
  union {
    int primitives_offset;
    int second_child_offset;
  };

  uint16_t num_primitives;
  uint8_t axis;
  uint8_t pad;
};

class BVH {
 public:
  BVH() = default;

  void build(std::vector<std::shared_ptr<Hittable>>& objects);
  bool intersect(Ray& ray, HitRecord& rec) const;
  bool is_occluded(const Ray& ray) const;
  AABB get_root_aabb();

 private:
  std::vector<LinearBvhNode> nodes_;
  std::vector<std::shared_ptr<Hittable>> primitives_;

  int flatten_bvh_tree(BvhBuildNode* node, int* offset);
};

//#pragma once
//
//#include <algorithm>
//#include <iterator>
//#include <memory>
//#include <random>
//#include <vector>
//
//#include "core/aabb.h"
//#include "core/ray.h"
//#include "hittable.h"
//
//class BvhNode : public Hittable {
// public:
//  template <typename Iterator>
//  BvhNode(Iterator begin, Iterator end);
//
//  bool local_intersect(Ray& ray, HitRecord& rec) const override;
//
//  AABB get_aabb() const override;
//
// private:
//  std::shared_ptr<Hittable> left_;
//  std::shared_ptr<Hittable> right_;
//  AABB bounding_box_;
//};
//
//template <typename Iterator>
//BvhNode::BvhNode(Iterator begin, Iterator end) {
//  std::vector<std::shared_ptr<Hittable>> objects(begin, end);
//
//  size_t object_span = objects.size();
//  AABB centroid_box;
//  for (const auto& obj : objects) {
//    centroid_box.expand(obj->get_aabb().centroid());
//  }
//  int axis = centroid_box.longest_axis();
//
//  if (object_span == 1) {
//    left_ = right_ = objects[0];
//  } else if (object_span == 2) {
//    left_ = objects[0];
//    right_ = objects[1];
//  } else {
//    size_t mid = object_span / 2;
//    std::nth_element(objects.begin(), objects.begin() + mid, objects.end(),
//                     [axis](const std::shared_ptr<Hittable>& a,
//                            const std::shared_ptr<Hittable>& b) {
//                       return a->get_aabb().axis(axis).min <
//                              b->get_aabb().axis(axis).min;
//                     });
//    left_ = std::make_shared<BvhNode>(objects.begin(), objects.begin() + mid);
//    right_ = std::make_shared<BvhNode>(objects.begin() + mid, objects.end());
//  }
//
//  bounding_box_ = AABB(left_->get_aabb(), right_->get_aabb());
//}