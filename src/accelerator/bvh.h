#pragma once

#include <vector>
#include <memory>
#include "core/aabb.h"
#include "core/ray.h"
#include "hittable.h"

namespace hasmet {

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

struct BvhBuildNode {
  AABB bounds;
  BvhBuildNode* left = nullptr;
  BvhBuildNode* right = nullptr;
  int split_axis = 0;
  int first_prim_offset = 0;
  int num_primitives = 0;

  void init_leaf(int first, int n, const AABB& b) {
    first_prim_offset = first;
    num_primitives = n;
    bounds = b;
    left = right = nullptr;
  }

  void init_interior(int axis, BvhBuildNode* c0, BvhBuildNode* c1) {
    left = c0;
    right = c1;
    split_axis = axis;
    bounds = AABB(c0->bounds, c1->bounds);
    num_primitives = 0;
  }
};

template <typename T>
BvhBuildNode* recursive_build(std::vector<T>& objects,
                              int start, int end) {
  BvhBuildNode* node = new BvhBuildNode();
  AABB bounds;

  if (start < end) bounds = objects[start].get_aabb();
  for (int i = start + 1; i < end; ++i) {
    bounds.expand(objects[i].get_aabb());
  }

  int num_primitives = end - start;
  if (num_primitives == 1) {
    node->init_leaf(start, num_primitives, bounds);
    return node;
  }

  AABB centroid_bounds = AABB();
  for (int i = start; i < end; ++i) {
    centroid_bounds.expand(objects[i].get_aabb().centroid());
  }

  int dim = centroid_bounds.longest_axis();
  int mid = (start + end) / 2;

  if (centroid_bounds.x.max == centroid_bounds.x.min &&
      centroid_bounds.y.max == centroid_bounds.y.min &&
      centroid_bounds.z.max == centroid_bounds.z.min) {
    node->init_leaf(start, num_primitives, bounds);
    return node;
  }

  std::nth_element(&objects[start], &objects[mid], &objects[end - 1] + 1,
                   [dim](const T& a,
                         const T& b) {
                     return a.get_aabb().centroid()[dim] <
                            b.get_aabb().centroid()[dim];
                   });
  node->init_interior(dim, recursive_build(objects, start, mid),
                      recursive_build(objects, mid, end));
  return node;
}

template <typename T>
class BVH {
 public:
  BVH() = default;

  void build(std::vector<T>& objects) {
    if (objects.empty()) return;

    primitives_ = objects;
    BvhBuildNode* root = recursive_build(primitives_, 0, primitives_.size());

    nodes_.resize(primitives_.size() * 2);
    int offset = 0;
    flatten_bvh_tree(root, &offset);
    nodes_.resize(offset);
  }

  bool intersect(Ray& ray, HitRecord& rec) const {
    if (nodes_.empty()) return false;

    bool hit = false;
    int to_visit_offset = 0;
    int current_node_index = 0;
    int nodes_to_visit[64];

    while (true) {
      const LinearBvhNode* node = &nodes_[current_node_index];

      if (node->bbox.intersect(ray)) {
        if (node->num_primitives > 0) {
          // Leaf node
          for (int i = 0; i < node->num_primitives; i++) {
            if (primitives_[node->primitives_offset + i].intersect(ray, rec)) {
              hit = true;
              ray.t_max = rec.t;
            }
          }
          if (to_visit_offset == 0) break;
          current_node_index = nodes_to_visit[--to_visit_offset];
        } else {
          // Heuristic (choose the one which is closer to the ray)
          if (ray.direction[node->axis] > 0) {
            nodes_to_visit[to_visit_offset++] = node->second_child_offset;
            current_node_index = current_node_index + 1;
          } else {
            nodes_to_visit[to_visit_offset++] = current_node_index + 1;
            current_node_index = node->second_child_offset;
          }
        }
      } else {
        if (to_visit_offset == 0) break;
        current_node_index = nodes_to_visit[--to_visit_offset];
      }
    }
    return hit;
  }

  bool is_occluded(const Ray& ray) const {
    if (nodes_.empty()) return false;

    Ray shadow_ray = ray;

    int to_visit_offset = 0;
    int current_node_index = 0;
    int nodes_to_visit[64];

    while (true) {
      const LinearBvhNode* node = &nodes_[current_node_index];

      if (node->bbox.intersect(shadow_ray)) {
        if (node->num_primitives > 0) {
          // Leaf
          for (int i = 0; i < node->num_primitives; ++i) {
            HitRecord temp_rec;
            if (primitives_[node->primitives_offset + i].intersect(shadow_ray,
                                                                    temp_rec)) {
              return true;
            }
          }
          if (to_visit_offset == 0) break;
          current_node_index = nodes_to_visit[--to_visit_offset];
        } else {
          // Interior node
          if (shadow_ray.direction[node->axis] > 0) {
            nodes_to_visit[to_visit_offset++] = node->second_child_offset;
            current_node_index = current_node_index + 1;
          } else {
            nodes_to_visit[to_visit_offset++] = current_node_index + 1;
            current_node_index = node->second_child_offset;
          }
        }
      } else {
        if (to_visit_offset == 0) break;
        current_node_index = nodes_to_visit[--to_visit_offset];
      }
    }
    return false;
  }

  AABB get_root_aabb() const { return nodes_[0].bbox; }

 private:
  std::vector<LinearBvhNode> nodes_;
  std::vector<T> primitives_;

  int flatten_bvh_tree(BvhBuildNode* node, int* offset) {
    LinearBvhNode* linear_node = &nodes_[*offset];
    linear_node->bbox = node->bounds;
    int my_offset = (*offset)++;

    if (node->num_primitives > 0) {
      linear_node->primitives_offset = node->first_prim_offset;
      linear_node->num_primitives = node->num_primitives;
    } else {
      linear_node->axis = node->split_axis;
      linear_node->num_primitives = 0;
      flatten_bvh_tree(node->left, offset);
      linear_node->second_child_offset = flatten_bvh_tree(node->right, offset);
    }
    delete node;

    return my_offset;
  }
};
} // namespace hasmet