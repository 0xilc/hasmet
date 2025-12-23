#include "triangle.h"
#include <glm/glm.hpp>
#include "core/types.h"

namespace hasmet {
Triangle::Triangle(const Vec3 vertices[3], const Vec3 vertex_normals[3], const Vec2 tex_coords[3], const Vec3 tangents[3], bool smooth_shading) {
  vertices_[0] = vertices[0];
  vertices_[1] = vertices[1];
  vertices_[2] = vertices[2];
  smooth_shading_ = smooth_shading;

  if (smooth_shading_) {
    vertex_normals_[0] = vertex_normals[0];
    vertex_normals_[1] = vertex_normals[1];
    vertex_normals_[2] = vertex_normals[2];
  }

  if (tex_coords) {
    has_uvs_ = true;
    tex_coords_[0] = tex_coords[0];
    tex_coords_[1] = tex_coords[1];
    tex_coords_[2] = tex_coords[2];
  } else {
    has_uvs_ = false;
  }

  if (tangents) {
    has_tangents_ = true;
    tangents_[0] = tangents[0];
    tangents_[1] = tangents[1];
    tangents_[2] = tangents[2];
  } else {
    has_tangents_ = false;
  }

  Vec3 min_v = glm::min(glm::min(vertices_[0], vertices_[1]), vertices_[2]);
  Vec3 max_v = glm::max(glm::max(vertices_[0], vertices_[1]), vertices_[2]);
  local_aabb_ = AABB(min_v, max_v);
}

bool Triangle::intersect(Ray& ray, HitRecord& rec) const {
  Vec3 edge1 = vertices_[1] - vertices_[0];
  Vec3 edge2 = vertices_[2] - vertices_[0];

  Vec3 h = glm::cross(ray.direction, edge2);
  float a = glm::dot(edge1, h);

  if (a > -1e-8 && a < 1e-8) return false;

  float f = 1.0f / a;
  Vec3 s = ray.origin - vertices_[0];
  
  float u = f * glm::dot(s, h);

  if (u < 0.0f || u > 1.0f) return false;

  Vec3 q = glm::cross(s, edge1);
  float v = f * glm::dot(ray.direction, q);

  if (v < 0.0f || u + v > 1.0f) return false;

  float t = f * glm::dot(edge2, q);

  if (t > ray.t_min && t < ray.t_max) {
    rec.t = t;
    rec.p = ray.origin + ray.direction * t;

    if (smooth_shading_) {
      rec.normal =
          glm::normalize((1.0f - u - v) * vertex_normals_[0] +
                         u * vertex_normals_[1] + v * vertex_normals_[2]);
    } else {
      rec.normal = glm::normalize(glm::cross(edge1, edge2));
    }

    if (has_uvs_) {
      rec.uv = (1.0f - u - v) * tex_coords_[0] +
                u * tex_coords_[1] + v * tex_coords_[2];
    } else {
      rec.uv = Vec2(0.0f);
    }

    if (has_tangents_) {
      Vec3 interpolated_tangent = (1.0f - u - v) * tangents_[0] +
                                   u * tangents_[1] +
                                   v * tangents_[2];
      rec.tangent = glm::normalize(interpolated_tangent);
    } else {
      rec.tangent = Vec3(0.0f);
    }

    return true;
  }
  return false;
}

AABB Triangle::get_aabb() const { return local_aabb_; }
} // namespace hasmet