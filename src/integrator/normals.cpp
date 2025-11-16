// src/integrators/normals.cpp

#include "normals.h"

#include <cmath>
#include <iostream>

#include "camera/pinhole.h"
#include "core/logging.h"
#include "film/film.h"
#include "scene/scene.h"

Normals::Normals(int max_depth) : max_depth_(max_depth) {}

void Normals::render(const Scene& scene, Film& film,
                     const Camera& camera) const {
  int width = film.getWidth();
  int height = film.getHeight();

#pragma omp parallel for
  for (int y = 0; y < height - 1; ++y) {
    for (int x = 0; x < width; ++x) {
      Ray r = camera.generateRay(static_cast<float>(x), static_cast<float>(y));
      Color pixel_color = Li(r, scene, max_depth_);
      film.addSample(x, y, pixel_color);
    }
  }
}

Color Normals::Li(Ray& ray, const Scene& scene, int depth) const {
  HitRecord rec;
  if (!scene.intersect(ray, rec)) return scene.render_config_.background_color;

  glm::vec3 color = glm::normalize(rec.normal) * 255.f;

  return Color(color.x, color.y, color.z);
}
