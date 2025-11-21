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