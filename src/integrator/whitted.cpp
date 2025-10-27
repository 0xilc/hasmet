// src/integrators/whitted.cpp

#include "whitted.h"

#include <cmath>
#include <iostream>

#include "camera/pinhole.h"
#include "core/logging.h"
#include "film/film.h"
#include "material/material.h"
#include "scene/scene.h"

Spectrum calculate_blinn_phong(const HitRecord& rec, const Scene& scene,
                               const glm::vec3& view_dir);

WhittedIntegrator::WhittedIntegrator(int max_depth) : max_depth_(max_depth) {}

void WhittedIntegrator::render(const Scene& scene, Film& film) const {
  int width = film.getWidth();
  int height = film.getHeight();

  PinholeCamera camera(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0),
                       glm::vec3(0, -1, 0), 90.0f, width, height);

  LOG_INFO("Rendering scene with Whitted integrator...");
  for (int y = height - 1; y >= 0; --y) {
    if (y % 10 == 0) {
      std::cout << "\rScanlines remaining: " << y << ' ' << std::flush;
    }
    for (int x = 0; x < width; ++x) {
      Ray r = camera.generateRay(static_cast<float>(x), static_cast<float>(y));
      Spectrum pixel_color = Li(r, scene, max_depth_);
      film.addSample(x, y, pixel_color);
    }
  }
  std::cout << "\nDone." << std::endl;
}

Spectrum WhittedIntegrator::Li(const Ray& ray, const Scene& scene,
                               int depth) const {
  if (depth <= 0) {
    return Spectrum(0.0f);
  }

  HitRecord rec;
  if (!scene.intersect(ray, rec)) {
    return Spectrum(0.4f, 0.1, 0.1f);
  }

  const Material& mat = *rec.mat_ptr;
  glm::vec3 final_color(0.0f);
  glm::vec3 view_dir = glm::normalize(-ray.direction);

  final_color += calculate_blinn_phong(rec, scene, view_dir);

  switch (mat.type) {
    case MaterialType::Mirror: {
      // Reflection ray.
      glm::vec3 reflected_dir = glm::reflect(ray.direction, rec.normal);
      Ray reflected_ray(rec.p, reflected_dir);

      final_color +=
          mat.mirror_reflectance * Li(reflected_ray, scene, depth - 1);
      break;
    }
    case MaterialType::Dielectric:
    case MaterialType::BlinnPhong:
    case MaterialType::Conductor:
    default:
      break;
  }

  return Spectrum(final_color.x, final_color.y, final_color.z);
}

Spectrum calculate_blinn_phong(const HitRecord& rec, const Scene& scene,
                               const glm::vec3& view_dir) {
  Spectrum color(0.0f);
  Material& material = *rec.mat_ptr;

  for (const auto& p_light : scene.point_lights_) {
    glm::vec3 light_dir = p_light->position - rec.p;
    float distance_to_light = glm::length(light_dir);
    light_dir = glm::normalize(light_dir);

    // Shadow test
    Ray shadow_ray(rec.p, light_dir);
    HitRecord shadow_rec;
    shadow_ray.tmax = distance_to_light;
    if (scene.intersect(shadow_ray, shadow_rec)) {
      continue;
    }

    // Diffuse component
    float cos_theta = std::max(0.0f, glm::dot(rec.normal, light_dir));
    Spectrum diffuse = material.diffuse_reflectance * cos_theta;

    // Specular component
    glm::vec3 halfway_dir = glm::normalize(light_dir + view_dir);
    float cos_alpha = std::max(0.0f, glm::dot(rec.normal, halfway_dir));
    Spectrum specular =
        material.specular_reflectance * pow(cos_alpha, material.phong_exponent);

    Spectrum light_contribution = (diffuse + specular) * p_light->intensity /
                                  (distance_to_light * distance_to_light);
    color = color + light_contribution;
  }

  return color;
}