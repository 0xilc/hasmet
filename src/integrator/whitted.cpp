// src/integrators/whitted.cpp

#include "whitted.h"

#include <cmath>
#include <iostream>

#include "camera/pinhole.h"
#include "core/logging.h"
#include "film/film.h"
#include "material/material.h"
#include "material/material_manager.h"
#include "scene/scene.h"

namespace {
inline float fresnel_dielectric(float cosThetaI, float etaI, float etaT,

                                float cosThetaT) {
  float r_s = (etaT * cosThetaI - etaI * cosThetaT) /

              (etaT * cosThetaI + etaI * cosThetaT);

  float r_p = (etaI * cosThetaI - etaT * cosThetaT) /

              (etaI * cosThetaI + etaT * cosThetaT);

  return 0.5f * (r_s * r_s + r_p * r_p);
}

inline double fresnel_conductor(double cos_theta, double n, double k) {
  double r_s = ((n * n + k * k) - 2 * n * cos_theta + cos_theta * cos_theta) /
               ((n * n + k * k) + 2 * n * cos_theta + cos_theta * cos_theta);
  double r_p =
      ((n * n + k * k) * cos_theta * cos_theta - 2 * n * cos_theta + 1) /
      ((n * n + k * k) * cos_theta * cos_theta + 2 * n * cos_theta + 1);
  return 0.5 * (r_s + r_p);
}
}  // namespace

WhittedIntegrator::WhittedIntegrator(int max_depth) : max_depth_(max_depth) {}

void WhittedIntegrator::render(const Scene& scene, Film& film,
                               const Camera& camera) const {
  int width = film.getWidth();
  int height = film.getHeight();

  LOG_INFO("Rendering scene...");
  for (int y = 0; y < height - 1; ++y) {
    for (int x = 0; x < width; ++x) {
      if (x == 446 && y == 518) {
        int debug = 0;
      }
      Ray r = camera.generateRay(static_cast<float>(x), static_cast<float>(y));
      Color pixel_color = Li(r, scene, max_depth_);
      film.addSample(x, y, pixel_color);
    }
  }
  LOG_INFO("Rendering completed.");
}

Color WhittedIntegrator::Li(Ray& ray, const Scene& scene, int depth) const {
  if (depth <= 0) return Color(0.0f);

  HitRecord rec;
  if (!scene.intersect(ray, rec)) return scene.render_config_.background_color;

  MaterialManager* material_manager = MaterialManager::get_instance();
  const Material& mat = material_manager->get(rec.material_id);
  Color final_color(0.0f);

  float intersection_test_epsilon =
      scene.render_config_.intersection_test_epsilon;

  switch (mat.type) {
    case MaterialType::Mirror: {
      final_color += calculate_blinn_phong(ray, rec, scene);
      glm::vec3 wo = glm::normalize(ray.origin - rec.p);
      glm::vec3 wr = glm::normalize(glm::reflect(-wo, rec.normal));

      Ray reflected_ray =
          Ray(rec.p + rec.normal * intersection_test_epsilon, wr);

      final_color +=
          Li(reflected_ray, scene, depth - 1) * mat.mirror_reflectance;
      break;
    }

    case MaterialType::Dielectric: {
      bool front_face = glm::dot(ray.direction, rec.normal) < 0.0f;
      glm::vec3 Nf = front_face ? rec.normal : -rec.normal;

      if (front_face) {
        final_color += calculate_blinn_phong(ray, rec, scene);
      }
      float n_1 = 1.0f;
      float n_2 = mat.refraction_index;
      if (!front_face) std::swap(n_1, n_2);

      float eta = n_1 / n_2;
      glm::vec3 wo = glm::normalize(-ray.direction);
      float cos_theta = std::clamp(-glm::dot(Nf, wo), 0.0f, 1.0f);
      float sin2_phi = eta * eta * std::max(0.0f, 1.0f - cos_theta * cos_theta);

      // Total reflection
      if (sin2_phi >= 1.0f) {
        glm::vec3 wr = glm::reflect(wo, Nf);
        Ray reflected_ray(rec.p + Nf * intersection_test_epsilon, wr);
        final_color += Li(reflected_ray, scene, depth - 1);
        break;
      }

      // Reflection and Refraction
      double cos_phi = std::sqrt(std::max(0.0f, 1.0f - sin2_phi));
      double Fr = fresnel_dielectric(cos_theta, n_1, n_2, cos_phi);
      double Ft = 1.0 - Fr;

      // Reflection
      glm::vec3 wr = glm::normalize(glm::reflect(-wo, rec.normal));
      Ray reflected_ray = Ray(rec.p + Nf * intersection_test_epsilon, wr);

      // Refraction
      glm::vec3 wt = glm::refract(wo, Nf, eta);
      Ray refracted_ray = Ray(rec.p - Nf * intersection_test_epsilon, wt);
      final_color += Fr * Li(reflected_ray, scene, depth - 1) +
                     Ft * Li(refracted_ray, scene, depth - 1);

      break;
    }

    case MaterialType::Conductor: {
      glm::vec3 wo = glm::normalize(ray.origin - rec.p);
      glm::vec3 wr = glm::normalize(glm::reflect(-wo, rec.normal));

      Ray reflected_ray =
          Ray(rec.p + rec.normal * intersection_test_epsilon, wr);

      double cos_theta = glm::dot(rec.normal, wr);
      double k = mat.absorption_index;
      double n = mat.refraction_index;
      double Fr = fresnel_conductor(cos_theta, n, k);
      final_color +=
          Fr * Li(reflected_ray, scene, depth - 1) * mat.mirror_reflectance;
      break;
    }

    case MaterialType::BlinnPhong: {
      final_color += calculate_blinn_phong(ray, rec, scene);
      break;
    }

    default:
      break;
  }

  return final_color;
}

Color WhittedIntegrator::calculate_blinn_phong(const Ray& ray,
                                               const HitRecord& rec,
                                               const Scene& scene) const {
  Color color(0.0f);
  MaterialManager* material_manager = MaterialManager::get_instance();
  const Material& material = material_manager->get(rec.material_id);
  float shadow_ray_epsilon = scene.render_config_.shadow_ray_epsilon;
  // Add ambient light
  color += material.ambient_reflectance * Color(scene.ambient_light_->radiance);
  for (const std::unique_ptr<PointLight>& light : scene.point_lights_) {
    glm::vec3 wi = light->position - rec.p;
    float distance_to_light = glm::length(wi);
    wi = glm::normalize(wi);

    // Shadow test
    Ray shadow_ray(rec.p + rec.normal * shadow_ray_epsilon, wi);
    HitRecord shadow_rec;
    shadow_ray.interval_.max = distance_to_light;
    if (scene.intersect(shadow_ray, shadow_rec)) {
      continue;
    }

    // Diffuse component
    double cos_theta = std::max(0.0f, glm::dot(rec.normal, wi));
    color += Color(material.diffuse_reflectance) * Color(light->intensity) *
             (cos_theta / (distance_to_light * distance_to_light));

    // Specular component
    glm::vec3 wo = glm::normalize(ray.origin - rec.p);
    glm::vec3 h = glm::normalize(wi + wo);
    double cos_alpha = std::max(0.0f, glm::dot(rec.normal, h));
    color += Color(material.specular_reflectance) * Color(light->intensity) *
             (std::pow(cos_alpha, material.phong_exponent) /
              (distance_to_light * distance_to_light));
  }

  return color;
}