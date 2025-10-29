// src/main.cpp

#include <memory>

#include "camera/pinhole.h"
#include "core/logging.h"
#include "film/film.h"
#include "geometry/sphere.h"
#include "integrator/whitted.h" 
#include "material/material.h"
#include "scene/scene.h"
#include "material/material_manager.h"


int main() {
  LOG_INFO("Hasmet Renderer | Assignment 1");

  // --- Image and Render Setup ---
  const int image_width = 1920;
  const int image_height = 1080;
  const std::string output_filename = "assignment_render.png";
  const int max_recursion_depth = 5;

  Film film(image_width, image_height, output_filename);

  Scene world;

  // A simple matte red material
  MaterialManager* material_manager = MaterialManager::get_instance();
  Material mat_red;
  mat_red.type = MaterialType::BlinnPhong;
  mat_red.diffuse_reflectance = Color(0.1f, 0.5f, 0.1f);
  mat_red.ambient_reflectance = mat_red.diffuse_reflectance;
  mat_red.specular_reflectance = Color(0.5f);
  mat_red.phong_exponent = 32.0f;
  int mat_red_id = material_manager->add(mat_red);

  // A gray, matte ground material
  Material mat_ground;
  mat_ground.type = MaterialType::BlinnPhong;
  mat_ground.diffuse_reflectance = Color(0.5f);
  mat_ground.ambient_reflectance = mat_ground.diffuse_reflectance;
  int mat_ground_id = material_manager->add(mat_ground);

  // A perfect mirror material
  Material mat_mirror;
  mat_mirror.type = MaterialType::Mirror;
  mat_mirror.mirror_reflectance = Color(0.9f);
  int mat_mirror_id = material_manager->add(mat_mirror);

  // A glass material
  Material mat_glass;
  mat_glass.type = MaterialType::Dielectric;
  mat_glass.refraction_index = 1.5;
  mat_glass.mirror_reflectance = Color(1.0f);
  int mat_glass_id = material_manager->add(mat_glass);

  world.add_ambient_light(std::make_unique<AmbientLight>(Color(0.1f)));
  world.add_point_light(
      std::make_unique<PointLight>(glm::vec3(2, 5, 2), Color(100.0f)));

  world.add_shape(
      std::make_unique<Sphere>(glm::vec3(0, -100.5, -1), 100.0f, mat_ground_id));
  world.add_shape(std::make_unique<Sphere>(glm::vec3(0, 0, -1), 0.5f, mat_red_id));
  world.add_shape(std::make_unique<Sphere>(glm::vec3(-1.2, 0, -1), 0.5f, mat_glass_id));
  world.add_shape(std::make_unique<Sphere>(glm::vec3(-1.2, 0, -2), 0.5f, mat_red_id));
  world.add_shape(
      std::make_unique<Sphere>(glm::vec3(1.2, 0, -1), 0.5f, mat_mirror_id));

  // Ray tracer rendering
  WhittedIntegrator integrator(max_recursion_depth);
  integrator.render(world, film);

  film.write();

  LOG_INFO("Render complete.");
  return 0;
}