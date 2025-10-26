// src/main.cpp

#include <memory>  // Required for std::make_unique

#include "camera/pinhole.h"
#include "core/logging.h"
#include "film/film.h"
#include "geometry/sphere.h"
#include "scene/scene.h"

Spectrum backgroundColor(const Ray& ray) {
  glm::vec3 unit_direction = normalize(ray.direction);
  float t = 0.5f * (unit_direction.y + 1.0f);
  Spectrum white(1.0f, 1.0f, 1.0f);
  Spectrum light_blue(0.5f, 0.7f, 1.0f);
  return (1.0f - t) * light_blue + t * white;
}

Spectrum ray_color(const Ray& r, const Scene& scene) {
  HitRecord rec;

  if (scene.intersect(r, rec)) {
    return 0.5f *
           Spectrum(rec.normal.x + 1, rec.normal.y + 1, rec.normal.z + 1);
  }

  return backgroundColor(r);
}

int main() {
  LOG_INFO("Hasmet Renderer | Phase 3");

  // --- Image Setup ---
  const int image_width = 800;
  const int image_height = 600;
  const std::string output_filename = "output.png";

  // --- World and Camera Setup ---
  Film film(image_width, image_height, output_filename);

  glm::vec3 camera_pos(0, 0, 1);
  glm::vec3 look_at(0, 0, 0);
  glm::vec3 up(0, 1, 0);
  float fov = 90.0f;
  PinholeCamera camera(camera_pos, look_at, up, fov, image_width, image_height);

  // --- Create the Scene and add objects to it ---
  Scene world;
  // Add a small sphere in the center of the view.
  world.add(std::make_unique<Sphere>(glm::vec3(0, 0, -1), 0.5f));
  // Add a very large sphere to act as the "ground".
  world.add(std::make_unique<Sphere>(glm::vec3(0, -100.5, -1), 100.0f));

  // --- Rendering Loop ---
  LOG_INFO("Rendering scene...");

  for (int y = image_height - 1; y >= 0; --y) {
    if (y % 10 == 0) {
      std::cout << "\rScanlines remaining: " << y << ' ' << std::flush;
    }
    for (int x = 0; x < image_width; ++x) {
      Ray r = camera.generateRay(static_cast<float>(x), static_cast<float>(y));
      // Calculate the color for the ray using our new function.
      Spectrum pixel_color = ray_color(r, world);
      film.addSample(x, y, pixel_color);
    }
  }
  std::cout << "\nDone." << std::endl;

  // --- Save the Final Image ---
  film.write();

  LOG_INFO("Render complete.");
  return 0;
}