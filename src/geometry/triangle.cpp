#include "triangle.h"

#include <glm/glm.hpp>

Triangle::Triangle(const glm::vec3& p1, const glm::vec3& p2,
                   const glm::vec3& p3, int material_id) {
  indices_[0] = p1;
  indices_[1] = p2;
  indices_[2] = p3;
  material_id_ = material_id;

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
    rec.normal = vec1;
    rec.p = ray.origin + ray.direction * static_cast<float>(t);
    rec.material_id = material_id_;
    return true;
  }
  return false;
}

AABB Triangle::getAABB() const { return aabb_; }