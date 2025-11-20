#include "triangle.h"

#include <glm/glm.hpp>

Triangle::Triangle(const glm::vec3& p1, const glm::vec3& p2,
                   const glm::vec3& p3, int material_id,
                   const glm::vec3 vertex_normals[3], bool smooth_shading) {
  indices_[0] = p1;
  indices_[1] = p2;
  indices_[2] = p3;
  material_id_ = material_id;
  smooth_shading_ = smooth_shading;
  if (smooth_shading_) {
    vertex_normals_[0] = vertex_normals[0];
    vertex_normals_[1] = vertex_normals[1];
    vertex_normals_[2] = vertex_normals[2];
  }

  glm::vec3 min_v = glm::min(glm::min(indices_[0], indices_[1]), indices_[2]);
  glm::vec3 max_v = glm::max(glm::max(indices_[0], indices_[1]), indices_[2]);
  aabb_ = AABB(min_v, max_v);
}

bool Triangle::local_intersect(Ray& ray, HitRecord& rec) const {
  glm::vec3 edge1 = indices_[1] - indices_[0];
  glm::vec3 edge2 = indices_[2] - indices_[0];

  glm::vec3 h = glm::cross(ray.direction, edge2);
  float a = glm::dot(edge1, h);

  if (a == 0) return false;

  float f = 1.0f / a;
  glm::vec3 s = ray.origin - indices_[0];
  float u = f * glm::dot(s, h);

  if (u < 0.0f || u > 1.0f) return false;

  glm::vec3 q = glm::cross(s, edge1);
  float v = f * glm::dot(ray.direction, q);

  if (v < 0.0f || u + v > 1.0f) return false;

  float t = f * glm::dot(edge2, q);

  if (t > ray.interval_.min && t < ray.interval_.max) {
    rec.t = t;
    rec.p = ray.origin + ray.direction * t;
    rec.material_id = material_id_;

    if (smooth_shading_) {
      rec.normal =
          glm::normalize((1.0f - u - v) * vertex_normals_[0] +
                         u * vertex_normals_[1] + v * vertex_normals_[2]);
    } else {
      rec.normal = glm::normalize(glm::cross(edge1, edge2));
    }
    return true;
  }
  return false;
}

AABB Triangle::getAABB() const { return aabb_; }