// src/integrators/whitted.cpp

#include "whitted.h"

#include <cmath>

#include "core/hit_record.h"
#include "core/sampler.h"
#include "core/timer.h"
#include "core/types.h"
#include "film/film.h"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/glm.hpp"
#include "light/light.h"
#include "material/bxdf.h"
#include "material/material.h"
#include "scene/scene.h"
#include "texture/texture_manager.h"

namespace hasmet {
struct ShadingContext {
  const Ray &ray;
  const HitRecord &rec;
  const Material &mat;
  Sampler &sampler;
  int pixel_id;
  int sample_index;
  int num_samples;
};

namespace {
inline bool is_same_hemisphere(const glm::vec3 &v1, glm::vec3 &v2) {
  return (glm::dot(v1, v2) > 0);
}
} // namespace

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

          Ray ray = camera.generateRay(static_cast<float>(x),
                                       static_cast<float>(y), u_pixel, u_lens);
          ray.time = time_sample;

          pixel_color +=
              trace_ray(ray, scene, scene.render_context_.max_recursion_depth,
                        local_sampler, s, camera.num_samples_, pixel_id);
        }

        film.addSample(x, y,
                       pixel_color / static_cast<float>(camera.num_samples_));
      }
    }
  }
}

Color WhittedIntegrator::trace_ray(Ray &ray, const Scene &scene, int depth,
                                   Sampler &sampler, int sample_index,
                                   int num_samples, int pixel_id) const {
  if (depth <= 0)
    return Color(0.0f);

  HitRecord rec;
  if (!scene.intersect(ray, rec)) {
    if (!scene.environment_light_) {
      return scene.render_context_.background_color;
    }
    return scene.environment_light_->sample_le(ray);
    ;
  }

  const Material &mat = *scene.get_material(rec.material_id);
  ShadingContext ctx{ray,      rec,          mat,        sampler,
                     pixel_id, sample_index, num_samples};
  BSDF bsdf(rec);
  mat.setup_bsdf(rec, bsdf);

  Vec3 woW = -glm::normalize(ray.direction);
  Color L(0.0f);

  L += shade_direct(bsdf, rec, woW, scene, sampler, sample_index, pixel_id);

  Vec2 u = sampler.get_2d(pixel_id, sample_index, depth + 100);
  BxDFSample bs = bsdf.sample_f(woW, u);

  if (bs.pdf > 0 && (bs.sampled_type & BSDF_SPECULAR)) {
    Vec3 offset_dir =
        (is_same_hemisphere(bs.wi, rec.normal)) ? rec.normal : -rec.normal;
    Ray next_ray(rec.p + (offset_dir * 0.00006f), bs.wi);
    next_ray.time = ray.time;

    Color L_recursive = trace_ray(next_ray, scene, depth - 1, sampler,
                                  sample_index, num_samples, pixel_id);

    L += bs.f * L_recursive * std::abs(glm::dot(bs.wi, rec.normal)) / bs.pdf;
  }

  return L;
}

Color WhittedIntegrator::shade_direct(const BSDF &bsdf, const HitRecord &rec,
                                      const Vec3 &woW, const Scene &scene,
                                      Sampler &sampler, int sample_idx,
                                      int pid) const {

  Color L_direct(0.0f);

  auto process_lights = [&](const auto& light_list) {
    for (const auto& light : light_list) {
      glm::vec2 u = sampler.get_2d(pid, sample_idx, 0);
      LightSample ls = light->sample_li(rec, u);

      if (ls.pdf <= 0.0f || glm::length(ls.L)) continue;

      Ray shadow_ray(rec.p + rec.normal * 0.0006f, ls.wi);
      shadow_ray.t_max = ls.dist - 0.0006f;

      if (scene.is_occluded(shadow_ray)) continue;
      
      Color f = bsdf.f(woW, ls.wi);
      float cos_theta = std::max(0.0f, glm::dot(rec.normal, ls.wi));

      L_direct += (f * ls.L * cos_theta) / ls.pdf;
    }
  };

  process_lights(scene.point_lights_);
  process_lights(scene.area_lights_);
  process_lights(scene.spot_lights_);
  process_lights(scene.directional_lights_);

  return L_direct;
}
} // namespace hasmet