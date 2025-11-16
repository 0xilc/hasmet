#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "camera/pinhole.h"
#include "geometry/mesh.h"
#include "geometry/plane.h"
#include "geometry/sphere.h"
#include "geometry/triangle.h"
#include "material/material_manager.h"
#include "parser/parser.h"
#include "scene/scene.h"

namespace Parser::ParserAdapter {

glm::mat4 create_transformation_matrix(
    const std::vector<Parser::Transformation_>& transforms) {
  glm::mat4 composite_matrix = glm::mat4(1.0f);

  for (const auto& tf : transforms) {
    glm::mat4 t_matrix = glm::mat4(1.0f);
    switch (tf.type) {
      case Parser::TransformationType::TRANSLATION:
        t_matrix = glm::translate(
            glm::mat4(1.0f), glm::vec3(tf.data[0], tf.data[1], tf.data[2]));
        break;
      case Parser::TransformationType::SCALING:
        t_matrix = glm::scale(glm::mat4(1.0f),
                              glm::vec3(tf.data[0], tf.data[1], tf.data[2]));
        break;
      case Parser::TransformationType::ROTATION:
        t_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(tf.data[0]),
                               glm::vec3(tf.data[1], tf.data[2], tf.data[3]));
        break;
      case Parser::TransformationType::COMPOSITE:
        t_matrix = glm::transpose(glm::make_mat4(tf.data.data()));
        break;
    }
    composite_matrix = t_matrix * composite_matrix;
  }
  return composite_matrix;
}

float get_triangle_area(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
  glm::vec3 edge1 = v2 - v1;
  glm::vec3 edge2 = v3 - v1;
  glm::vec3 cross_product = glm::cross(edge1, edge2);
  return 0.5f * glm::length(cross_product);
}

Color create_color(const Parser::Vec3f_ v_) { return Color(v_.x, v_.y, v_.z); }

glm::vec3 create_vec3(const Parser::Vec3f_& v_) {
  return glm::vec3(v_.x, v_.y, v_.z);
}

Sphere create_sphere(const Parser::Sphere_& sphere_,
                     const std::vector<Parser::Vec3f_>& vertex_data_) {
  return Sphere(create_vec3(vertex_data_[sphere_.center_vertex_id]),
                sphere_.radius, sphere_.material_id);
}

Triangle create_triangle(const Parser::Triangle_& triangle_,
                         const std::vector<Parser::Vec3f_>& vertex_data_,
                         const glm::vec3 vertex_normals[3] = nullptr,
                         bool smooth_shading = false) {
  return Triangle(create_vec3(vertex_data_[triangle_.v0_id]),
                  create_vec3(vertex_data_[triangle_.v1_id]),
                  create_vec3(vertex_data_[triangle_.v2_id]),
                  triangle_.material_id, vertex_normals, smooth_shading);
}

PointLight create_point_light(const Parser::PointLight_ light_) {
  return PointLight{create_vec3(light_.position),
                    create_color(light_.intensity)};
}

Material create_material(const Parser::Material_& material_) {
  Material mat;

  std::string type_str = material_.type;
  std::transform(type_str.begin(), type_str.end(), type_str.begin(), ::tolower);

  if (type_str == "mirror")
    mat.type = MaterialType::Mirror;
  else if (type_str == "conductor")
    mat.type = MaterialType::Conductor;
  else if (type_str == "dielectric")
    mat.type = MaterialType::Dielectric;
  else
    mat.type = MaterialType::BlinnPhong;

  mat.ambient_reflectance = create_color(material_.ambient_reflectance);
  mat.diffuse_reflectance = create_color(material_.diffuse_reflectance);
  mat.specular_reflectance = create_color(material_.specular_reflectance);
  mat.mirror_reflectance = create_color(material_.mirror_reflectance);
  mat.absorption_coefficient = create_color(material_.absorption_coefficient);
  mat.absorption_index = material_.absorption_index;
  mat.phong_exponent = material_.phong_exponent;
  mat.refraction_index = material_.refraction_index;

  return mat;
}

PinholeCamera create_pinhole_camera(const Parser::Camera_& camera_) {
  glm::vec3 position(camera_.position.x, camera_.position.y,
                     camera_.position.z);
  glm::vec3 gaze(camera_.gaze.x, camera_.gaze.y, camera_.gaze.z);
  glm::vec3 up(camera_.up.x, camera_.up.y, camera_.up.z);

  glm::vec3 look_at = position + gaze;

  float top = camera_.near_plane.t;
  float bottom = camera_.near_plane.b;

  float vertical_fov_radians =
      2.0f * atan((top - bottom) * 0.5f / camera_.near_distance);
  float vertical_fov_degrees = glm::degrees(vertical_fov_radians);

  return PinholeCamera(position, look_at, up, vertical_fov_degrees,
                       camera_.image_width, camera_.image_height,
                       camera_.image_name);
}

Scene read_scene(std::string filename) {
  Parser::Scene_ parsed_scene;
  Parser::parseScene(filename, parsed_scene);

  Scene scene;
  scene.render_config_ = RenderConfig{
      create_color(parsed_scene.background_color),
      parsed_scene.shadow_ray_epsilon, parsed_scene.intersection_test_epsilon,
      parsed_scene.max_recursion_depth};

  for (const Parser::Camera_& camera_ : parsed_scene.cameras) {
    scene.cameras_.push_back(
        std::make_unique<PinholeCamera>(create_pinhole_camera(camera_)));
  }

  MaterialManager* material_manager = MaterialManager::get_instance();
  for (const Parser::Material_ material_ : parsed_scene.materials) {
    material_manager->add(material_.id, create_material(material_));
  }

  for (const Parser::Sphere_& sphere_ : parsed_scene.spheres) {
    auto sphere_obj = std::make_unique<Sphere>(
        create_sphere(sphere_, parsed_scene.vertex_data));
  }

  for (const Parser::PointLight_& light_ : parsed_scene.point_lights) {
    scene.point_lights_.push_back(
        std::make_unique<PointLight>(create_point_light(light_)));
  }

  scene.ambient_light_ =
      std::make_unique<AmbientLight>(create_color(parsed_scene.ambient_light));

  for (const Parser::Triangle_& triangle_ : parsed_scene.triangles) {
    scene.objects_.push_back(std::make_unique<Triangle>(
        create_triangle(triangle_, parsed_scene.vertex_data)));
  }

  std::unordered_map<int, Mesh*> mesh_map;
  
  for (const Parser::Mesh_& mesh_ : parsed_scene.meshes) {
    std::vector<std::shared_ptr<Triangle>> mesh_faces;
    if (mesh_.smooth_shading) {
      std::vector<std::vector<std::pair<glm::vec3, float>>>
          per_vertex_triangles;
      per_vertex_triangles.resize(parsed_scene.vertex_data.size());
      for (const Triangle_& triangle_ : mesh_.faces) {
        glm::vec3 v0 = create_vec3(parsed_scene.vertex_data[triangle_.v0_id]);
        glm::vec3 v1 = create_vec3(parsed_scene.vertex_data[triangle_.v1_id]);
        glm::vec3 v2 = create_vec3(parsed_scene.vertex_data[triangle_.v2_id]);
        float area = get_triangle_area(v0, v1, v2);
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 face_normal = glm::normalize(glm::cross(edge1, edge2));
        per_vertex_triangles[triangle_.v0_id].push_back(
            std::make_pair(face_normal, area));
        per_vertex_triangles[triangle_.v1_id].push_back(
            std::make_pair(face_normal, area));
        per_vertex_triangles[triangle_.v2_id].push_back(
            std::make_pair(face_normal, area));
      }
      std::vector<glm::vec3> vertex_normals;
      for (const auto& v : per_vertex_triangles) {
        glm::vec3 normal(0.0, 0.0, 0.0);
        float total_area = 0.0;
        for (const auto& pair : v) {
          normal = normal + pair.first * pair.second;
          total_area += pair.second;
        }
        if (total_area > 0.0) {
          normal = normal / total_area;
        }
        vertex_normals.push_back(normal);
      }
      for (const Triangle_& triangle_ : mesh_.faces) {
        glm::vec3 indices[3] = {
            create_vec3(parsed_scene.vertex_data[triangle_.v0_id]),
            create_vec3(parsed_scene.vertex_data[triangle_.v1_id]),
            create_vec3(parsed_scene.vertex_data[triangle_.v2_id])};

        glm::vec3 per_vertex_normals[3] = {vertex_normals[triangle_.v0_id],
                                           vertex_normals[triangle_.v1_id],
                                           vertex_normals[triangle_.v2_id]};

        mesh_faces.push_back(std::make_shared<Triangle>(
            indices[0], indices[1], indices[2], triangle_.material_id,
            per_vertex_normals, true));
      }
    } else {
      for (const Parser::Triangle_& face_ : mesh_.faces) {
        mesh_faces.push_back(std::make_unique<Triangle>(
            create_triangle(face_, parsed_scene.vertex_data)));
      }
    }

    std::shared_ptr<Mesh> mesh = std::make_unique<Mesh>(mesh_faces, mesh_.material_id);
    scene.objects_.push_back(std::move(mesh));
    mesh_map[mesh_.id] = static_cast<Mesh*>(scene.objects_.back().get());
  }

  for (const Parser::MeshInstance_ mi_ : parsed_scene.mesh_instances) {
    // TODO: Fix ordering issue here. if the base mesh is not defined before
    // the instance, it will cause error.
    const Mesh* base_mesh_ = mesh_map[mi_.base_mesh_id];
    if (base_mesh_ == nullptr) {
      std::cerr << "Error: Base mesh with ID " << mi_.base_mesh_id
                << " not found for mesh instance " << mi_.id << std::endl;
      continue;
    }
    std::shared_ptr<BvhNode> blas = base_mesh_->blas_;
    std::shared_ptr<Mesh> mesh_instance =
        std::make_shared<Mesh>(blas, mi_.material_id);
    scene.objects_.push_back(std::move(mesh_instance));
    mesh_map[mi_.id] = static_cast<Mesh*>(scene.objects_.back().get());
  }
  
  for (const Parser::Plane_& plane_ : parsed_scene.planes) {
    glm::vec3 point =
        create_vec3(parsed_scene.vertex_data[plane_.point_vertex_id]);
    glm::vec3 normal = create_vec3(plane_.normal);
    scene.objects_.push_back(
        std::make_unique<Plane>(point, normal, plane_.material_id));
  }

  scene.build_bvh();
  return scene;
}

}  // namespace Parser::ParserAdapter
