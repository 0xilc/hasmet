#include "pathtracer.h"
#include "core/hit_record.h"
#include "core/sampling.h"
#include "core/timer.h"
#include "core/types.h"
#include "film/film.h"
#include "glm/geometric.hpp"
#include "integrator/whitted.h"
#include "material/bxdf.h"
#include "scene/scene.h"
#include <cmath>
#include "core/frame.h"

namespace hasmet {

void PathTracerIntegrator::configure(const std::vector<std::string>& params) {
  for (const auto& p : params) {
    if (p == "NextEventEstimation") config_.use_nee = true;
    else if (p == "ImportanceSampling") config_.use_importance = true;
    else if (p == "RussianRoulette") config_.use_rr = true;
    else if (p == "MIS_BALANCE" || p == "MIS_balance") {
      config_.use_mis = true;
      config_.mis_heuristic = MISHeuristic::Balance;
    } else if (p == "MIS_POWER" || p == "MIS_power") {
      config_.use_mis = true;
      config_.mis_heuristic = MISHeuristic::Power;
    } else if (p == "MIS_01") {
      config_.use_mis = true;
      config_.mis_heuristic = MISHeuristic::ZeroOne;
    }
  }
}

namespace {
inline float luminance(const Color& c) {
  return 0.2126f * c.r + 0.7152f * c.g + 0.0722f * c.b;
}

inline float power_heuristic(float pdf_a, float pdf_b) {
  return (pdf_a * pdf_a) / (pdf_a * pdf_a + pdf_b * pdf_b);
}

inline float balance_heuristic(float pdf_a, float pdf_b) {
    return pdf_a / (pdf_a + pdf_b);
}

inline float mis_01(float pdf_a, float pdf_b) {
    return (pdf_a > 0) ? 1.0f : 0.0f;
}
}

float PathTracerIntegrator::mis_weight(float pdf_a, float pdf_b) const {
  switch (config_.mis_heuristic) {
    case MISHeuristic::Power:   return power_heuristic(pdf_a, pdf_b);
    case MISHeuristic::ZeroOne: return mis_01(pdf_a, pdf_b);
    default:                    return balance_heuristic(pdf_a, pdf_b);
  }
}

Vec3 local_to_world(const Vec3 &local_vector, const Vec3 &normal) {
  Vec3 up = (std::abs(normal.y) < 0.999) ? Vec3(0, 1, 0) : Vec3(1, 0, 0);
  Vec3 tangent = normalize(cross(up, normal));
  Vec3 binormal = cross(normal, tangent);

  return tangent * local_vector.x + binormal * local_vector.y +
         normal * local_vector.z;
}

void PathTracerIntegrator::render(const Scene &scene, Film &film,
                                  const Camera &camera) const {
  SCOPED_TIMER("Render: [Path Tracing]");
  int width = film.getWidth();
  int height = film.getHeight();
  int samples_per_pixel = camera.num_samples_;
  int max_depth = scene.render_context_.max_recursion_depth
                      ? scene.render_context_.max_recursion_depth
                      : 6;
#pragma omp parallel
  {
    Sampler local_sampler;
#pragma omp for schedule(dynamic, 10)
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        Color pixel_color(0.0f);
        int pixel_id = y * width + x;

        for (int s = 0; s < camera.num_samples_; s++) {
          SamplingContext ctx{local_sampler, pixel_id, s, camera.num_samples_};
          
          glm::vec2 u_pixel = local_sampler.get_2d(pixel_id, s, 0);
          glm::vec2 u_lens = local_sampler.get_2d(pixel_id, s, 1);

          Ray ray = camera.generateRay(static_cast<float>(x), static_cast<float>(y), u_pixel, u_lens);
          
          pixel_color += trace_path(ray, scene, ctx, max_depth);
        }
        film.addSample(x, y, pixel_color / static_cast<float>(camera.num_samples_));
      }
    }
  }
}

Color PathTracerIntegrator::trace_path(Ray &ray, const Scene &scene, SamplingContext& ctx, int max_depth) const {
    Color L(0.0f);
    Color throughput(1.0f);
    bool is_specular_bounce = true;
    float prev_bsdf_pdf = 0.0f;

    for (int depth = 0; depth < max_depth; ++depth) {
        HitRecord rec;
        if (!scene.intersect(ray, rec)) {
          if (scene.environment_light_) {
            L += throughput * scene.environment_light_->sample_le(ray);
          }
          break;
        }

        const Material &mat = *scene.get_material(rec.material_id);
        BSDF bsdf(rec);
        mat.setup_bsdf(rec, bsdf);
        Vec3 woW = -glm::normalize(ray.direction);

        // Emission from hitting a light surface
        if (rec.radiance.has_value()) {
          Color emission = rec.radiance.value();
          if (!config_.use_nee || is_specular_bounce) {
            // No NEE or specular bounce: full weight (NEE can't sample delta dirs)
            L += throughput * emission;
          } else if (config_.use_mis) {
            // MIS: weight BSDF-sampled hit against what NEE would have given
            float light_pdf = scene.light_pdf(ray, rec);
            float weight = (light_pdf > 0) ? mis_weight(prev_bsdf_pdf, light_pdf) : 1.0f;
            L += throughput * emission * weight;
          }
          // If NEE is on but MIS is off: skip emission (NEE handles it)
        }
        
        L += throughput * estimate_direct(scene, bsdf, rec, woW, ctx, depth);
        // Direct lighting via NEE
        if (config_.use_nee) {
          L += throughput * estimate_direct(scene, bsdf, rec, woW, ctx, depth);
        }

        Vec2 u = ctx.sampler.get_2d(ctx.pixel_id, ctx.sample_index, depth + 10);
        BxDFSample bs = bsdf.sample_f(woW, u);

        if (bs.pdf <= 0.0f || luminance(bs.f) < 1e-8f) break;

        float cos_theta = std::abs(glm::dot(bs.wi, rec.normal));
        throughput *= (bs.f * cos_theta) / bs.pdf;
        prev_bsdf_pdf = bs.pdf;

        is_specular_bounce = (bs.sampled_type & BSDF_SPECULAR) != 0;

        if (config_.use_rr && depth >= 3) {
          float p_live = std::min(luminance(throughput), 0.99f);
          if (ctx.sampler.get_1d(ctx.pixel_id, ctx.sample_index, depth + 50) > p_live) break;
          throughput /= p_live;
        }

        ray = Ray(rec.p + bs.wi * 0.0001f, bs.wi);
    }

    return L;
}

Color PathTracerIntegrator::estimate_direct(const Scene& scene, const BSDF& bsdf, const HitRecord& rec, const Vec3& woW, SamplingContext& ctx, int depth) const {
  Color Ld(0.0f);

  int num_point = static_cast<int>(scene.point_lights_.size());
  int num_area = static_cast<int>(scene.area_lights_.size());
  int num_spot = static_cast<int>(scene.spot_lights_.size());
  int num_object = static_cast<int>(scene.light_indices_.size());
  int total_lights = num_point + num_area + num_spot + num_object;
  if (total_lights == 0) return Ld;

  float light_pick_pdf = 1.0f / total_lights;
  int rand_idx = static_cast<int>(ctx.sampler.get_1d(ctx.pixel_id, ctx.sample_index, depth + 200) * total_lights);
  rand_idx = std::min(rand_idx, total_lights - 1);

  Vec2 u_light = ctx.sampler.get_2d(ctx.pixel_id, ctx.sample_index, depth + 300);
  LightSample ls;
  bool is_delta_light = false;

  int offset = 0;
  if (rand_idx < num_point) {
    ls = scene.point_lights_[rand_idx]->sample_li(rec, u_light);
    is_delta_light = true;
  } else if (rand_idx < (offset = num_point) + num_area) {
    ls = scene.area_lights_[rand_idx - offset]->sample_li(rec, u_light);
  } else if (rand_idx < (offset = num_point + num_area) + num_spot) {
    ls = scene.spot_lights_[rand_idx - offset]->sample_li(rec, u_light);
    is_delta_light = true;
  } else {
    int obj_idx = scene.light_indices_[rand_idx - num_point - num_area - num_spot];
    ls = scene.objects_[obj_idx].sample_li(rec, u_light);
  }

  if (ls.pdf > 0 && luminance(ls.L) > 1e-8f) {
      Vec3 bias_normal = glm::dot(rec.normal, ls.wi) > 0 ? rec.normal : -rec.normal;
      Ray shadow_ray(rec.p + bias_normal * 1e-4f, ls.wi);
      shadow_ray.t_max = ls.dist - 2e-4f;

      if (!scene.is_occluded(shadow_ray)) {
          float cos_theta = std::max(0.0f, glm::dot(rec.normal, ls.wi));
          Color f = bsdf.f(woW, ls.wi);

          float light_pdf = ls.pdf * light_pick_pdf;
          float weight = 1.0f;

          if (!is_delta_light && config_.use_mis) {
            float bsdf_pdf = bsdf.pdf(woW, ls.wi);
            weight = mis_weight(light_pdf, bsdf_pdf);
          }

          Ld = (f * ls.L * cos_theta * weight) / light_pdf;
      }
  }

  return Ld;
}
} // namespace hasmet