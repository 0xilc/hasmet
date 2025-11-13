#include "accelerator/bvh.h"

bool BvhNode::intersect(Ray& ray, HitRecord& rec) const {
  if (!bounding_box_.intersect(ray)) {
    return false;
  }

  bool hit_left = left_->intersect(ray, rec);
  bool hit_right = right_->intersect(ray, rec);
  return hit_left || hit_right;
}

AABB BvhNode::getAABB() const { return bounding_box_; }
