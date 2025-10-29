#pragma once

#include <iostream>
#include "camera/pinhole.h"
#include "geometry/sphere.h"
#include "geometry/triangle.h"
#include "geometry/plane.h"
#include "material/material_manager.h"
#include "parser/parser.h"
#include "scene/scene.h"

namespace Parser::ParserAdapter {

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
                         const std::vector<Parser::Vec3f_>& vertex_data_) {
  return Triangle(create_vec3(vertex_data_[triangle_.v0_id]),
                  create_vec3(vertex_data_[triangle_.v1_id]),
                  create_vec3(vertex_data_[triangle_.v2_id]),
                  triangle_.material_id);
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
  mat.absorption_index = create_color(material_.absorption_index);

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
                       camera_.image_width, camera_.image_height);
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
    scene.objects_.push_back(std::make_unique<Sphere>(
        create_sphere(sphere_, parsed_scene.vertex_data)));
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
  
  for (const Parser::Mesh_& mesh_ : parsed_scene.meshes) {
    for (const Parser::Triangle_& face_ : mesh_.faces) {
      scene.objects_.push_back(std::make_unique<Triangle>(
          create_triangle(face_, parsed_scene.vertex_data)));
    }
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
