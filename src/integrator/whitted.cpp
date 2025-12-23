// src/integrators/whitted.cpp

#include "whitted.h"

#include <cmath>
#include <iostream>

#include "camera/pinhole.h"
#include "core/logging.h"
#include "core/sampler.h"
#include "core/sampling.h"
#include "core/types.h"
#include "film/film.h"
#include "material/material.h"
#include "material/material_manager.h"
#include "texture/texture_manager.h"
#include "scene/scene.h"

namespace hasmet {
namespace {
inline float fresnel_dielectric(float cosThetaI, float etaI, float etaT,

                                float cosThetaT) {
  float r_s = (etaT * cosThetaI - etaI * cosThetaT) /

              (etaT * cosThetaI + etaI * cosThetaT);

  float r_p = (etaI * cosThetaI - etaT * cosThetaT) /

              (etaI * cosThetaI + etaT * cosThetaT);

  return 0.5f * (r_s * r_s + r_p * r_p);
}

inline float fresnel_conductor(float cos_theta, float n, float k) {
  float r_s = ((n * n + k * k) - 2 * n * cos_theta + cos_theta * cos_theta) /
              ((n * n + k * k) + 2 * n * cos_theta + cos_theta * cos_theta);
  float r_p =
      ((n * n + k * k) * cos_theta * cos_theta - 2 * n * cos_theta + 1) /
      ((n * n + k * k) * cos_theta * cos_theta + 2 * n * cos_theta + 1);
  return 0.5 * (r_s + r_p);
}
}  // namespace

namespace {
Material apply_textures(const Material& base_mat, HitRecord& rec) {
  if (rec.texture_ids == nullptr || rec.texture_ids->empty()){
    return base_mat;
  }

  Material mat = base_mat;
  TextureManager* tm = TextureManager::get_instance();

  for (int tid : *rec.texture_ids) {
    const Texture& tex = tm->get(tid);
    Color tex_color = tex.evaluate(rec.uv, rec.p);
    switch (tex.decal_mode) {
      case DecalMode::REPLACE_KD:
        mat.diffuse_reflectance = tex_color;
        break;
      
      case DecalMode::BLEND_KD:
        mat.diffuse_reflectance = (mat.diffuse_reflectance + tex_color) * 0.5f;
        break;
      
      case DecalMode::REPLACE_KS:
        mat.specular_reflectance = tex_color;
        break;
      
      case DecalMode::REPLACE_ALL:
        mat.diffuse_reflectance = tex_color * 255.0f;
        mat.type = MaterialType::TextureColor;
        break;
      
      case DecalMode::REPLACE_NORMAL:
        Vec3 map_normal = tex_color;
        map_normal = map_normal * 2.0f - Vec3(1.0f);
        map_normal.y = -map_normal.y;
        
        Vec3 N = glm::normalize(rec.normal);
        Vec3 T = rec.tangent;

        if (glm::length(T) < 1e-4) {
          break;
        }

        T = glm::normalize(T);
        T = glm::normalize(T - glm::dot(T, N) * N);
        Vec3 B = glm::cross(N, T);

        Mat3 TBN = Mat3(T, B, N);
        rec.normal = glm::normalize(TBN * map_normal);
        break;
    }
  }

  return mat;
}

void generate_area_light_samples(const AreaLight &light, int num_samples,
                                 std::vector<Vec3> &out) {
  out.clear();

  const auto &jittered_samples =
      Sampling::generate_jittered_samples(num_samples);

  for (const auto &[dx, dy] : jittered_samples) {
    Vec3 sample_point =
        light.position +
        (light.u * (dx - 0.5f) + light.v * (dy - 0.5f)) * light.size;

    out.emplace_back(sample_point);
  }
}

void perturb_ray(Ray &ray, float roughness) {
  Vec3 w = glm::normalize(ray.direction);

  Vec3 tmp =
      (std::abs(w.x) > 0.9f) ? Vec3(0.0f, 1.0f, 0.0f) : Vec3(1.0f, 0.0f, 0.0f);
  Vec3 u = glm::normalize(glm::cross(tmp, w));
  Vec3 v = glm::cross(w, u);

  float r1 = Sampling::_generate_random_float(0.0f, 1.0f) - 0.5;
  float r2 = Sampling::_generate_random_float(0.0f, 1.0f) - 0.5;

  Vec3 new_direction = ray.direction + (u * r1 + v * r2) * roughness;
  ray.direction = glm::normalize(new_direction);
}

}  // namespace

WhittedIntegrator::WhittedIntegrator(int max_depth) : max_depth_(max_depth) {}

void WhittedIntegrator::render(const Scene &scene, Film &film,
                               const Camera &camera) const {
  int width = film.getWidth();
  int height = film.getHeight();

#pragma omp parallel
  {
    Sampler local_sampler;

    #pragma omp for schedule(dynamic, 10)
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        Color pixel_color(0.0f);
        int pixel_id = y * width + x;

        for (int s = 0; s < camera.num_samples_; s++) {
          glm::vec2 u_pixel = local_sampler.get_2d(pixel_id, s, 0);
          glm::vec2 u_lens = local_sampler.get_2d(pixel_id, s, 1);
          float time_sample = local_sampler.get_1d(pixel_id, s, 2);
          
          #define CENTER_RAY 1
          #if CENTER_RAY
          u_pixel = Vec2(0.f);
          u_lens = Vec2(0.f);
          #endif

          Ray ray = camera.generateRay((float)x, float(y), u_pixel, u_lens);
          ray.time = time_sample;

          pixel_color += Li(ray, scene, max_depth_, s, camera.num_samples_);
        }

        film.addSample(x, y,
                       pixel_color / static_cast<float>(camera.num_samples_));
      }
    }
  }
}

Color WhittedIntegrator::Li(Ray &ray, const Scene &scene, int depth,
                            int sample_index, int num_samples) const {
  if (depth <= 0) return Color(0.0f);

  HitRecord rec;
  if (!scene.intersect(ray, rec)) return scene.render_config_.background_color;

  MaterialManager *material_manager = MaterialManager::get_instance();
  const Material &base_mat = material_manager->get(rec.material_id);

  Material mat = apply_textures(base_mat, rec);

  
  Color final_color(0.0f);

  float intersection_test_epsilon =
      scene.render_config_.intersection_test_epsilon;

  switch (mat.type) {
    case MaterialType::Mirror: {
      final_color += calculate_blinn_phong(ray, rec, mat, scene, depth, sample_index,
                                           num_samples);
      Vec3 wo = glm::normalize(ray.origin - rec.p);
      Vec3 wr = glm::normalize(glm::reflect(-wo, rec.normal));

      Ray reflected_ray =
          Ray(rec.p + rec.normal * intersection_test_epsilon, wr);

      if (mat.roughness) {
        perturb_ray(reflected_ray, mat.roughness);
      }

      final_color +=
          Li(reflected_ray, scene, depth - 1, sample_index, num_samples) *
          mat.mirror_reflectance;
      break;
    }

    case MaterialType::Dielectric: {
      Vec3 wo = glm::normalize(ray.origin - rec.p);
      bool entering = glm::dot(wo, rec.normal) > 0.0f;
      Vec3 normal = entering ? rec.normal : -rec.normal;

      float etaI = entering ? 1.0f : mat.refraction_index;
      float etaT = entering ? mat.refraction_index : 1.0f;
      float eta = etaI / etaT;

      float cosThetaI = glm::dot(wo, normal);
      float sin2ThetaI = std::max(0.0f, 1.0f - cosThetaI * cosThetaI);
      float sin2ThetaT = eta * eta * sin2ThetaI;

      Color reflect_color(0.0f);
      Color refract_color(0.0f);

      if (entering) {
        final_color += calculate_blinn_phong(ray, rec, mat, scene, depth,
                                             sample_index, num_samples);
      }

      // Total internal reflection
      if (sin2ThetaT >= 1.0f) {
        Vec3 wr = glm::reflect(-wo, normal);
        Ray reflected_ray(rec.p + normal * intersection_test_epsilon, wr);
        if (mat.roughness) {
          perturb_ray(reflected_ray, mat.roughness);
        }

        reflect_color =
            Li(reflected_ray, scene, depth - 1, sample_index, num_samples);
        final_color += reflect_color;
        break;
      }

      float cosThetaT = std::sqrt(std::max(0.0f, 1.0f - sin2ThetaT));

      float Fr = fresnel_dielectric(cosThetaI, etaI, etaT, cosThetaT);

      // Reflection
      Vec3 wr = glm::reflect(-wo, normal);
      Ray reflected_ray(rec.p + normal * intersection_test_epsilon, wr);
      if (mat.roughness) {
        perturb_ray(reflected_ray, mat.roughness);
      }

      reflect_color =
          Li(reflected_ray, scene, depth - 1, sample_index, num_samples);

      // Refraction
      Vec3 wt = eta * -wo + (eta * cosThetaI - cosThetaT) * normal;
      Ray refracted_ray(rec.p - normal * intersection_test_epsilon,
                        glm::normalize(wt));

      if (mat.roughness) {
        perturb_ray(refracted_ray, mat.roughness);
      }

      refract_color =
          Li(refracted_ray, scene, depth - 1, sample_index, num_samples);

      Color L0 = Fr * reflect_color + (1.0f - Fr) * refract_color;
      if (!entering) {
        float d = rec.t;
        L0.r *= std::exp(-mat.absorption_coefficient.r * d);
        L0.g *= std::exp(-mat.absorption_coefficient.g * d);
        L0.b *= std::exp(-mat.absorption_coefficient.b * d);
      }

      final_color += L0;
      break;
    }
    case MaterialType::Conductor: {
      Vec3 wo = glm::normalize(ray.origin - rec.p);
      Vec3 wr = glm::normalize(glm::reflect(-wo, rec.normal));

      Ray reflected_ray =
          Ray(rec.p + rec.normal * intersection_test_epsilon, wr);
      if (mat.roughness) {
        perturb_ray(reflected_ray, mat.roughness);
      }

      float NdotR = glm::dot(reflected_ray.direction, rec.normal);
      if (NdotR < 0.0f) {
        reflected_ray.direction =
            reflected_ray.direction - 2.0f * NdotR * rec.normal;
      }

      float cos_theta = glm::dot(rec.normal, wr);
      float k = mat.absorption_index;
      float n = mat.refraction_index;
      float Fr = fresnel_conductor(cos_theta, n, k);
      final_color += calculate_blinn_phong(ray, rec, mat, scene, depth, sample_index,
                                           num_samples);

      final_color +=
          Fr * Li(reflected_ray, scene, depth - 1, sample_index, num_samples) *
          mat.mirror_reflectance;
      break;
    }
    case MaterialType::BlinnPhong: {
      final_color += calculate_blinn_phong(ray, rec, mat, scene, depth, sample_index,
                                           num_samples);
      break;
    }
    case MaterialType::TextureColor: {
      final_color = mat.diffuse_reflectance;
    }

    default:
      break;
  }

  return final_color;
}

Color WhittedIntegrator::calculate_blinn_phong(const Ray &ray,
                                               const HitRecord &rec,
                                               const Material& material,
                                               const Scene &scene, int depth,
                                               int sample_index,
                                               int num_samples) const {
  int num_samples_sqrt = static_cast<int>(glm::sqrt(num_samples));
  Color color(0.0f);
  float shadow_ray_epsilon = scene.render_config_.shadow_ray_epsilon;

  // Add ambient light
  color += material.ambient_reflectance * Color(scene.ambient_light_->radiance);

  // Point Lights
  for (const std::unique_ptr<PointLight> &light : scene.point_lights_) {
    Vec3 wi = light->position - rec.p;
    float distance_to_light = glm::length(wi);
    wi = glm::normalize(wi);

    // Shadow test
    Ray shadow_ray(rec.p + rec.normal * shadow_ray_epsilon, wi);
    shadow_ray.t_max = distance_to_light - 1e-4f;
    shadow_ray.time = ray.time;

    if (glm::dot(rec.normal, wi) < 0.0f || scene.is_occluded(shadow_ray)) {
      continue;
    }

    // Diffuse component
    float cos_theta = std::max(0.0f, glm::dot(rec.normal, wi));
    color += Color(material.diffuse_reflectance) * Color(light->intensity) *
             (cos_theta / (distance_to_light * distance_to_light));

    // Specular component
    Vec3 wo = glm::normalize(ray.origin - rec.p);
    Vec3 h = glm::normalize(wi + wo);
    float cos_alpha = std::max(0.0f, glm::dot(rec.normal, h));
    color += Color(material.specular_reflectance) * Color(light->intensity) *
             (std::pow(cos_alpha, material.phong_exponent) /
              (distance_to_light * distance_to_light));
  }

  // Area Lights
  for (const auto &light : scene.area_lights_) {

    int ix = sample_index % num_samples_sqrt;
    int iy = sample_index / num_samples_sqrt;

    float r1 =
        (static_cast<float>(ix) + Sampling::_generate_random_float(0, 1)) /
        static_cast<float>(num_samples_sqrt);
    float r2 =
        (static_cast<float>(iy) + Sampling::_generate_random_float(0, 1)) /
        static_cast<float>(num_samples_sqrt);

    Vec3 sampled_light_point =
        light->position +
        (light->u * (r1 - 0.5f) + light->v * (r2 - 0.5f)) * light->size;
    Vec3 wi = sampled_light_point - rec.p;
    float distance_to_light = glm::length(wi);
    wi = glm::normalize(wi);

    float shadow_ray_epsilon = scene.render_config_.shadow_ray_epsilon;

    // Shadow test
    Ray area_shadow_ray(rec.p + rec.normal * shadow_ray_epsilon, wi);
    area_shadow_ray.t_max = distance_to_light - shadow_ray_epsilon;
    area_shadow_ray.time = ray.time;

    if (glm::dot(rec.normal, wi) < 0.0f || scene.is_occluded(area_shadow_ray))
      continue;

    // Attenuation
    float area_of_light = light->size * light->size;
    float cos_alpha_light = std::abs(glm::dot(-wi, light->normal));
    float dist2 = distance_to_light * distance_to_light;
    float attenuation = area_of_light * cos_alpha_light / dist2;

    // Diffuse component
    float cos_theta = std::max(0.0f, glm::dot(rec.normal, wi));
    color += Color(material.diffuse_reflectance) * Color(light->radiance) *
             cos_theta * attenuation;

    // Specular component
    Vec3 wo = glm::normalize(ray.origin - rec.p);
    Vec3 h = glm::normalize(wi + wo);
    float cos_alpha = std::max(0.0f, glm::dot(rec.normal, h));
    color += Color(material.specular_reflectance) * Color(light->radiance) *
             (std::pow(cos_alpha, material.phong_exponent) * attenuation);
  }

  return color;
}
}  // namespace hasmet