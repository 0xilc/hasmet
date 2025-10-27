// src/main.cpp

#include <memory>

#include "camera/pinhole.h"
#include "core/logging.h"
#include "film/film.h"
#include "geometry/sphere.h"
#include "integrator/whitted.h" 
#include "material/material.h"
#include "scene/scene.h"

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
  Material* mat_red = world.add_material(std::make_unique<Material>());
  mat_red->type = MaterialType::BlinnPhong;
  mat_red->diffuse_reflectance = Spectrum(0.1f, 0.5f, 0.1f);
  mat_red->ambient_reflectance =
      mat_red->diffuse_reflectance;
  mat_red->specular_reflectance = Spectrum(0.5f);
  mat_red->phong_exponent = 32.0f;

  // A gray, matte ground material
  auto mat_ground = world.add_material(std::make_unique<Material>());
  mat_ground->type = MaterialType::BlinnPhong;
  mat_ground->diffuse_reflectance = Spectrum(0.5f);
  mat_ground->ambient_reflectance = mat_ground->diffuse_reflectance;

  // A perfect mirror material
  auto mat_mirror = world.add_material(std::make_unique<Material>());
  mat_mirror->type = MaterialType::Mirror;
  mat_mirror->mirror_reflectance = Spectrum(0.9f);

  // A glass material
  auto mat_glass = world.add_material(std::make_unique<Material>());
  mat_glass->type = MaterialType::Dielectric;
  mat_glass->refraction_index = 1.5;
  mat_glass->mirror_reflectance = Spectrum(1.0f);

  world.add_ambient_light(std::make_unique<AmbientLight>(Spectrum(0.1f)));
  world.add_point_light(
      std::make_unique<PointLight>(glm::vec3(2, 5, 2), Spectrum(100.0f)));

  world.add_shape(
      std::make_unique<Sphere>(glm::vec3(0, -100.5, -1), 100.0f, mat_ground));
  world.add_shape(std::make_unique<Sphere>(glm::vec3(0, 0, -1), 0.5f, mat_red));
  world.add_shape(std::make_unique<Sphere>(glm::vec3(-1.2, 0, -1), 0.5f, mat_glass));
  world.add_shape(std::make_unique<Sphere>(glm::vec3(-1.2, 0, -2), 0.5f, mat_red));
  world.add_shape(
      std::make_unique<Sphere>(glm::vec3(1.2, 0, -1), 0.5f, mat_mirror));

  // Ray tracer rendering
  WhittedIntegrator integrator(max_recursion_depth);
  integrator.render(world, film);

  film.write();

  LOG_INFO("Render complete.");
  return 0;
}