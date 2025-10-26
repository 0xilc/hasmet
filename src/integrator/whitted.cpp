#include "whitted.h"

#include <iostream>

#include "camera/pinhole.h"
#include "core/logging.h"
#include "film/film.h"
#include "scene/scene.h"

WhittedIntegrator::WhittedIntegrator(int max_depth) : max_depth_(max_depth) {}

void WhittedIntegrator::render(const Scene& scene, Film& film) const {
  int width = film.getWidth();
  int height = film.getHeight();

  PinholeCamera camera(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0),
                       glm::vec3(0, 1, 0), 90.0f, width, height);

  LOG_INFO("Rendering scene with Whitted integrator...");
  for (int y = height - 1; y >= 0; --y) {
    for (int x = 0; x < width; ++x) {
      Ray r = camera.generateRay(static_cast<float>(x), static_cast<float>(y));
      Spectrum pixel_color = Li(r, scene, max_depth_);
      film.addSample(x, y, pixel_color);
    }
  }
  LOG_INFO("Rendering completed")
}

Spectrum WhittedIntegrator::Li(const Ray& ray, const Scene& scene,
                               int depth) const {
  HitRecord rec;
  if (scene.intersect(ray, rec)) {
    return 0.5f *
           Spectrum(rec.normal.x + 1, rec.normal.y + 1, rec.normal.z + 1);
  } else {
    return Spectrum(0.5f, 0.7f, 1.0f);
  }
}