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
}  // namespace

WhittedIntegrator::WhittedIntegrator(int max_depth) : max_depth_(max_depth) {}

void WhittedIntegrator::render(const Scene& scene, Film& film,
                               const Camera& camera) const {
  int width = film.getWidth();
  int height = film.getHeight();

  LOG_INFO("Rendering scene...");
  for (int y = 0; y < height - 1; ++y) {
    for (int x = 0; x < width; ++x) {
      Ray r = camera.generateRay(static_cast<float>(x), static_cast<float>(y));
      Color pixel_color = Li(r, scene, max_depth_);
      film.addSample(x, y, pixel_color);
    }
  }
  std::cout << "\nDone." << std::endl;
}

Color WhittedIntegrator::Li(Ray& ray, const Scene& scene, int depth) const {
  if (depth <= 0) return Color(0.0f);

  HitRecord rec;
  if (!scene.intersect(ray, rec)) return scene.render_config_.background_color;

  MaterialManager* material_manager = MaterialManager::get_instance();
  const Material& mat = material_manager->get(rec.material_id);
  glm::vec3 final_color(0.0f);
  glm::vec3 view_dir = glm::normalize(-ray.direction);
  float intersection_test_epsilon =
      scene.render_config_.intersection_test_epsilon;

  final_color += calculate_blinn_phong(rec, scene, view_dir);

  switch (mat.type) {
    case MaterialType::Mirror: {
      glm::vec3 reflected_dir = glm::reflect(ray.direction, rec.normal);
      Ray reflected_ray(rec.p, reflected_dir);

      final_color +=
          mat.mirror_reflectance * Li(reflected_ray, scene, depth - 1);
      break;
    }
    case MaterialType::Dielectric: {
      glm::vec3 I = glm::normalize(ray.direction);
      glm::vec3 N = glm::normalize(rec.normal);
      bool front_face = glm::dot(I, N) < 0.0f;

      glm::vec3 Nf = front_face ? N : -N;

      float etaI = 1.0f;
      float etaT = mat.refraction_index;
      if (!front_face) std::swap(etaI, etaT);

      float eta = etaI / etaT;
      float cosI = glm::clamp(-glm::dot(I, Nf), 0.0f, 1.0f);
      float sin2T = eta * eta * std::max(0.0f, 1.0f - cosI * cosI);
      if (sin2T >= 1.0f) {
        glm::vec3 R = glm::reflect(I, Nf);
        Ray rRay(rec.p + Nf * intersection_test_epsilon, R);
        final_color += Li(rRay, scene, depth - 1);
        break;
      }

      float cosT = std::sqrt(std::max(0.0f, 1.0f - sin2T));

      // Exact Fresnel term
      float Fr = fresnel_dielectric(cosI, etaI, etaT, cosT);
      float Ft = 1.0f - Fr;

      // Reflection ray
      glm::vec3 R = glm::reflect(I, Nf);
      Ray rRay(rec.p + Nf * intersection_test_epsilon, R);

      // Refraction ray
      glm::vec3 T = glm::refract(I, Nf, eta);
      Ray tRay(rec.p - Nf * intersection_test_epsilon, T);

      Color LoR = Li(rRay, scene, depth - 1);
      Color LoT = Li(tRay, scene, depth - 1);

      // No Beer's law yet; add it later by multiplying LoT with transmittance
      final_color += Fr * LoR + Ft * LoT;
      break;
    }
    case MaterialType::Conductor:
    case MaterialType::BlinnPhong:
    default:
      break;
  }

  return Color(final_color.x, final_color.y, final_color.z);
}

 Color WhittedIntegrator::calculate_blinn_phong(const HitRecord& rec, const Scene& scene,
                            const glm::vec3& view_dir) const {
  Color color(0.0f);
  MaterialManager* material_manager = MaterialManager::get_instance();
  const Material& material = material_manager->get(rec.material_id);

  for (const auto& p_light : scene.point_lights_) {
    glm::vec3 light_dir = p_light->position - rec.p;
    float distance_to_light = glm::length(light_dir);
    light_dir = glm::normalize(light_dir);
    float shadow_ray_epsilon = scene.render_config_.shadow_ray_epsilon;

    // Shadow test
    Ray shadow_ray(rec.p + rec.normal * shadow_ray_epsilon, light_dir);
    HitRecord shadow_rec;
    shadow_ray.interval_.max = distance_to_light;
    if (scene.intersect(shadow_ray, shadow_rec)) {
      continue;
    }

    // Diffuse component
    float cos_theta = std::max(0.0f, glm::dot(rec.normal, light_dir));
    Color diffuse = material.diffuse_reflectance * cos_theta;

    // Specular component
    glm::vec3 halfway_dir = glm::normalize(light_dir + view_dir);
    float cos_alpha = std::max(0.0f, glm::dot(rec.normal, halfway_dir));
    Color specular =
        material.specular_reflectance * pow(cos_alpha, material.phong_exponent);

    Color light_contribution =
        (diffuse + specular) * p_light->intensity /
        (4 * 3.1415 * distance_to_light * distance_to_light);
    color = color + light_contribution;
  }

  return color;
}