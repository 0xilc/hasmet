#include "accelerator/bvh.h"

AABB BvhNode::get_aabb() const { return bounding_box_; }

bool BvhNode::local_intersect(Ray& ray, HitRecord& rec) const {
  if (!bounding_box_.intersect(ray)) {
    return false;
  }

  bool hit_left = left_->intersect(ray, rec);

  if(hit_left) {
    ray.interval_.max = rec.t;
  }

  bool hit_right = right_->intersect(ray, rec);
  
  return hit_left || hit_right;
}
