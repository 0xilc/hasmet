#include "triangle.h"
#include <glm/glm.hpp>

Triangle::Triangle(const glm::vec3& p1, const glm::vec3& p2,
                   const glm::vec3& p3) {
  indices[0] = p1;
  indices[1] = p2;
  indices[2] = p3;
}

bool Triangle::intersect(const Ray& ray, HitRecord& rec) const {
  glm::vec3 c1 = indices[0] - indices[1];
  glm::vec3 c2 = indices[0] - indices[2];
  glm::vec3 c3 = ray.direction;
  double detA = glm::determinant(glm::mat3(c1, c2, c3));
  if (detA == 0) return false;

  c1 = indices[0] - ray.origin;
  double beta = glm::determinant(glm::mat3(c1, c2, c3)) / detA;

  c2 = c1;
  c1 = indices[0] - indices[1];
  double gamma = glm::determinant(glm::mat3(c1, c2, c3)) / detA;

  c3 = c2;
  c2 = indices[0] - indices[2];
  double t = glm::determinant(glm::mat3(c1, c2, c3)) / detA;

  if (t < ray.tmin + 0.0001 || 0.0001 + t > ray.tmax) return false;

  if (beta + gamma <= 1 && beta + 0.00001 >= 0 && gamma + 0.00001 >= 0) {
    glm::vec3 vec1 = indices[1] - indices[0];
    glm::vec3 vec2 = indices[2] - indices[0];
    vec1 = glm::cross(vec1, vec2);
    vec1 = glm::normalize(vec1);

    rec.t = t;
    rec.normal = vec1;
    rec.p = ray.origin + ray.direction * static_cast<float>(t);
    // TODO: Implement this after material manager. rec.mat_ptr = mat_ptr;
    return true;
  }
  return false;
}