#include "accelerator/bvh.h"

AABB BvhNode::getAABB() const { return bounding_box_; }

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

  //HitRecord left_rec, right_rec;
  //bool hit_left = left_->intersect(ray, left_rec);
  //bool hit_right = right_->intersect(ray, right_rec);

  //if (hit_left && hit_right) {
  //  // Both children were hit, pick the closer one.
  //  if (left_rec.t < right_rec.t) {
  //    rec = left_rec;
  //  } else {
  //    rec = right_rec;
  //  }
  //  return true;
  //} else if (hit_left) {
  //  // Only the left child was hit.
  //  rec = left_rec;
  //  return true;
  //} else if (hit_right) {
  //  // Only the right child was hit.
  //  rec = right_rec;
  //  return true;
  //}

  //// No hit
  //return false;
}
