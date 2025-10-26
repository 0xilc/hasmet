// src/main.cpp

#include <memory>

#include "camera/pinhole.h"
#include "core/logging.h"
#include "film/film.h"
#include "geometry/sphere.h"
#include "integrator/whitted.h"  // Include the new integrator
#include "material/material.h"   // We need the Material struct and enum
#include "scene/scene.h"

int main() {
  LOG_INFO("Hasmet Renderer | Assignment 1");

  // --- Image and Render Setup ---
  const int image_width = 800;
  const int image_height = 600;
  const std::string output_filename = "assignment_render.png";
  const int max_recursion_depth = 5;

  Film film(image_width, image_height, output_filename);

  // The Scene will now own all the objects, materials, and lights.
  Scene world;

  // --- Create Materials and add them to the Scene ---
  // The scene takes ownership and returns a stable raw pointer.

  // A simple matte red material
  Material* mat_red = world.add_material(std::make_unique<Material>());
  mat_red->type = MaterialType::BlinnPhong;
  mat_red->diffuse_reflectance = Spectrum(0.7f, 0.1f, 0.1f);
  mat_red->ambient_reflectance =
      mat_red->diffuse_reflectance;  // Often the same
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
  mat_glass->mirror_reflectance = Spectrum(1.0f);  // For tinting reflection

  // --- Create and add Lights to the Scene ---
  world.add_ambient_light(std::make_unique<AmbientLight>(Spectrum(0.1f)));
  world.add_point_light(
      std::make_unique<PointLight>(glm::vec3(2, 5, 2), Spectrum(100.0f)));

  // --- Create Shapes and assign Materials ---
  world.add_shape(
      std::make_unique<Sphere>(glm::vec3(0, -100.5, -1), 100.0f, mat_ground));
  world.add_shape(std::make_unique<Sphere>(glm::vec3(0, 0, -1), 0.5f, mat_red));
  world.add_shape(
      std::make_unique<Sphere>(glm::vec3(-1.2, 0, -1), 0.5f, mat_glass));
  world.add_shape(
      std::make_unique<Sphere>(glm::vec3(1.2, 0, -1), 0.5f, mat_mirror));

  // --- Create the Integrator ---
  // The integrator contains the core rendering algorithm.
  WhittedIntegrator integrator(max_recursion_depth);

  // --- Render the Scene ---
  // The main render call is now clean and simple.
  // We pass the scene and the film to the integrator.
  // The integrator will handle the camera itself for now.
  integrator.render(world, film);

  // --- Save the Final Image ---
  film.write();

  LOG_INFO("Render complete.");
  return 0;
}