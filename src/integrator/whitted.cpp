// src/integrators/whitted.cpp

#include "whitted.h"

#include <cmath>

#include "core/hit_record.h"
#include "core/sampler.h"
#include "core/types.h"
#include "film/film.h"
#include "glm/geometric.hpp"
#include "light/light.h"
#include "material/material.h"
#include "material/material_manager.h"
#include "texture/texture_manager.h"
#include "scene/scene.h"
#include "core/timer.h"

namespace hasmet {
struct ShadingContext {
    const Ray& ray;
    const HitRecord& rec;
    const Material& mat;
    Sampler& sampler;
    int pixel_id;
    int sample_index;
    int num_samples;
};

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
        mat.diffuse_reflectance = tex_color;
        mat.type = MaterialType::Unlit;
        break;
      
      case DecalMode::REPLACE_NORMAL: {
        Vec3 map_normal = tex_color;
        map_normal = map_normal * 2.0f - Vec3(1.0f);
        map_normal = glm::normalize(map_normal);

        Vec3 N = glm::normalize(rec.normal);
        Vec3 T = rec.tangents[0];
        Vec3 B = rec.tangents[1];
        T = glm::normalize(T - N * glm::dot(N, T));
        B = glm::cross(T, N);
        
        Mat3 TBN = Mat3(T, B, N);
        rec.normal = glm::normalize(TBN * map_normal);
        break;
      }
      case DecalMode::BUMP_NORMAL: {
        Vec3 bump_map = tex_color;
        float height = (tex_color.r + tex_color.g + tex_color.b) / 3.0f;

        Vec3 N = glm::normalize(rec.normal);
        Vec3 T = rec.tangents[0]; // dp/du
        Vec3 B = rec.tangents[1]; // dp/dv

        T = glm::normalize(T - N * glm::dot(N, T));
        B = glm::cross(T, N);
        
        Vec2 gradients = tex.get_height_derivative(rec.uv, rec.p, T, B);

        float dh_du = gradients.x * tex.bump_factor;
        float dh_dv = gradients.y * tex.bump_factor;
        Vec3 dq_du = T + dh_du * N;
        Vec3 dq_dv = B + dh_dv * N;

        Vec3 bumped_normal = glm::cross(dq_dv, dq_du);
        rec.normal = glm::normalize(bumped_normal);
        break;
      }
    }
  }

  return mat;
}

bool is_black(const Color& c) {
    return c.r <= 0.0f && c.g <= 0.0f && c.b <= 0.0f;
}
}  // namespace

void WhittedIntegrator::render(const Scene &scene, Film &film,
                               const Camera &camera) const {
  SCOPED_TIMER("Rendering");
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

          Ray ray = camera.generateRay(static_cast<float>(x), static_cast<float>(y), u_pixel, u_lens);
          ray.time = time_sample;

          pixel_color += Li(ray, scene, scene.render_context_.max_recursion_depth, local_sampler, s, camera.num_samples_, pixel_id);
        }

        film.addSample(x, y, pixel_color / static_cast<float>(camera.num_samples_));
      }
    }
  }
}

Color WhittedIntegrator::Li(Ray& ray, const Scene& scene, int depth, Sampler& sampler, int sample_index, int num_samples, int pixel_id) const {
  if (depth <= 0) return Color(0.0f);

  HitRecord rec;
  
  if (!scene.intersect(ray, rec)) {
    Color background(0.0f);
    for (const auto& env : scene.environment_lights_) background += env->sample_le(ray);
    return (scene.environment_lights_.empty()) ? scene.render_context_.background_color : background;
  }

  const Material& base_mat = MaterialManager::get_instance()->get(rec.material_id);
  Material mat = apply_textures(base_mat, rec);
  ShadingContext ctx{ray, rec, mat, sampler, pixel_id, sample_index, num_samples};
  
  Color L;
  if (mat.type == MaterialType::Unlit) {
      L = mat.diffuse_reflectance;
  } else {
      L = shade_blinn_phong(ctx, scene);
  }

  int dim = 100;
  Vec3 wo = -glm::normalize(ray.direction);
  std::vector<BxDFSample> recursive_samples = mat.sample_f(wo, rec, sampler.get_2d(pixel_id, sample_index, dim++));
  
  for (const auto& bs : recursive_samples) {
    if (bs.is_valid && !is_black(bs.weight)) {
      Vec3 offset_dir = bs.is_transmission ? -rec.normal : rec.normal;
      Ray next_ray(rec.p + offset_dir * scene.render_context_.intersection_eps, bs.wi);
      next_ray.time = ray.time;

      Color L_recursive = Li(next_ray, scene, depth - 1, sampler, sample_index, num_samples, pixel_id);

      // Beers law
      if (mat.type == MaterialType::Dielectric && bs.is_transmission && glm::dot(wo, rec.normal) < 0.0f) {
        L_recursive.r *= std::exp(-mat.absorption_coefficient.r * rec.t);
        L_recursive.g *= std::exp(-mat.absorption_coefficient.g * rec.t);
        L_recursive.b *= std::exp(-mat.absorption_coefficient.b * rec.t);
      }

      L += bs.weight * L_recursive;
    }
  }

  return L;
}

Color WhittedIntegrator::shade_blinn_phong(const ShadingContext& ctx, const Scene& scene) const {
  Color L_direct(0.0f);
  const RenderContext& rc = scene.render_context_;
  Vec3 wo = -glm::normalize(ctx.ray.direction);

  L_direct += ctx.mat.ambient_reflectance * scene.ambient_light_->radiance;
  
  int light_dim = 200;
  auto process_list = [&](const auto& light_list) {
    for (const auto& light : light_list) {
      LightSample ls = light->sample_li(ctx.rec, ctx.sampler.get_2d(ctx.pixel_id, ctx.sample_index, light_dim++));

      if (ls.pdf <= 0.0f || is_black(ls.L)) continue;

      Ray shadow_ray(ctx.rec.p + ctx.rec.normal * rc.shadow_eps, ls.wi);
      shadow_ray.t_max = ls.dist - rc.shadow_eps;
      shadow_ray.time = ctx.ray.time;

      if (!scene.is_occluded(shadow_ray)) {
        float cos_theta = std::max(0.0f, glm::dot(ctx.rec.normal, ls.wi));
        L_direct += (ctx.mat.evaluate(ls.wi, wo, ctx.rec) * ls.L * cos_theta) / ls.pdf;
      }
    }
  };

  process_list(scene.point_lights_);
  process_list(scene.area_lights_);
  process_list(scene.spot_lights_);
  process_list(scene.directional_lights_);
  process_list(scene.environment_lights_);

  return L_direct;
}

} // namespace hasmet