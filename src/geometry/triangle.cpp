#include "triangle.h"

#include <glm/glm.hpp>

Triangle::Triangle(const glm::vec3& p1, const glm::vec3& p2,
                   const glm::vec3& p3, int material_id,
                   const glm::vec3 vertex_normals[3],
                   bool smooth_shading) {
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

bool Triangle::intersect(Ray& ray, HitRecord& rec) const {
  glm::vec3 c1 = indices_[0] - indices_[1];
  glm::vec3 c2 = indices_[0] - indices_[2];
  glm::vec3 c3 = ray.direction;
  double detA = glm::determinant(glm::mat3(c1, c2, c3));
  if (detA == 0) return false;

  c1 = indices_[0] - ray.origin;
  double beta = glm::determinant(glm::mat3(c1, c2, c3)) / detA;

  c2 = c1;
  c1 = indices_[0] - indices_[1];
  double gamma = glm::determinant(glm::mat3(c1, c2, c3)) / detA;

  c3 = c2;
  c2 = indices_[0] - indices_[2];
  double t = glm::determinant(glm::mat3(c1, c2, c3)) / detA;

  if (t < ray.interval_.min + 0.0001 || 0.0001 + t > ray.interval_.max)
    return false;

  if (beta + gamma <= 1 && beta + 0.00001 >= 0 && gamma + 0.00001 >= 0) {
    glm::vec3 vec1 = indices_[1] - indices_[0];
    glm::vec3 vec2 = indices_[2] - indices_[0];
    vec1 = glm::cross(vec1, vec2);
    vec1 = glm::normalize(vec1);

    rec.t = t;
    rec.p = ray.origin + ray.direction * static_cast<float>(t);
    rec.material_id = material_id_;

    if (smooth_shading_) {
      rec.normal = glm::normalize(
          static_cast<float>(1 - beta - gamma) * vertex_normals_[0] +
          static_cast<float>(beta) * vertex_normals_[1] +
          static_cast<float>(gamma) * vertex_normals_[2]);
    } else {
      rec.normal = vec1;
    }
    return true;
  }
  return false;
}

AABB Triangle::getAABB() const { return aabb_; }