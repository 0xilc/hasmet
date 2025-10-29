#include "bvh.h"

BvhNode::BvhNode(std::vector<std::shared_ptr<Hittable>>& objects, int begin,
                 int end) {
  int axis = rand() % 3;
  if (begin == end) {
    left_ = objects[begin];
    right_ = objects[begin];
  } else if (begin == end - 1) {
    left_ = objects[begin];
    right_ = objects[end];
  } else {
    std::sort(objects.begin() + begin, objects.begin() + end + 1,
              [axis](const std::shared_ptr<Hittable>& a,
                     const std::shared_ptr<Hittable> b) {
                return a->getAABB()[axis].min < b->getAABB()[axis].min;
              });
    int mid = (begin + end) / 2;
    left_ = std::make_shared<BvhNode>(objects, begin, mid);
    right_ = std::make_shared<BvhNode>(objects, mid, end);
  }

  bounding_box_ = AABB(left_->getAABB(), right_->getAABB());
}

bool BvhNode::intersect(Ray& ray, HitRecord& rec) const {
  if (!bounding_box_.intersect(ray)) return false;
  HitRecord rec1, rec2;

  bool hit_left = left_->intersect(ray, rec1);
  bool hit_right = right_->intersect(ray, rec2);

  if (hit_left && hit_right) {
    if (rec1.t < rec2.t) {
      rec = rec1;
    } else {
      rec = rec2;
    }
    return true;
  } else if (hit_left) {
    rec = rec1;
    return true;
  } else if (hit_right) {
    rec = rec2;
    return true;
  }

  return false;
}

AABB BvhNode::getAABB() const { return bounding_box_; }
