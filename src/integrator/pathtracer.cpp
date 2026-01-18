#include "pathtracer.h"
#include "core/hit_record.h"
#include "core/sampling.h"
#include "core/timer.h"
#include "core/types.h"
#include "film/film.h"
#include "glm/geometric.hpp"
#include "material/material_manager.h"
#include "scene/scene.h"
#include <cmath>
#include "core/frame.h"

namespace hasmet {

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
          glm::vec2 u_pixel = local_sampler.get_2d(pixel_id, s, 0);
          glm::vec2 u_lens = local_sampler.get_2d(pixel_id, s, 1);
          float time_sample = local_sampler.get_1d(pixel_id, s, 2);

          Ray ray = camera.generateRay(static_cast<float>(x),
                                       static_cast<float>(y), u_pixel, u_lens);
          ray.time = time_sample;

          pixel_color += trace_path(ray, scene, local_sampler, pixel_id, s, max_depth);
        }
        film.addSample(x, y,
                       pixel_color / static_cast<float>(camera.num_samples_));
      }
    }
  }
}

Color PathTracerIntegrator::trace_path(Ray &ray, const Scene &scene, Sampler& sampler, int pid, int sid, int max_depth) const {
    Color L(0.0f);
    Color throughput(1.0f);
    bool last_was_specular = true;
    float last_brdf_pdf = 1.0f;

    for (int depth = 0; depth < max_depth; ++depth) {
        HitRecord rec;
        if (!scene.intersect(ray, rec)) {
            if (!scene.environment_lights_.empty()) {
                L += throughput * scene.environment_lights_[0]->sample_le(ray);
            } else {
                L += throughput * scene.render_context_.background_color;
            }
            break;
        }

        Material &mat = MaterialManager::get_instance()->get(rec.material_id);
        Vec3 wo = -glm::normalize(ray.direction);

        // --- 1. DIRECT EMISSION ---
        if (rec.radiance.has_value()) {
            Color emission = rec.radiance.value();
            if (last_was_specular || !config_.use_mis) {
                L += throughput * emission;
            } else {
                float dist2 = rec.t * rec.t;
                float cos_light = std::max(0.0f, glm::dot(rec.normal, wo));
                float p_light = dist2 / (1.0f * cos_light); 
                float weight = balance_heuristic(last_brdf_pdf, p_light);
                L += throughput * emission * weight;
            }
            break; 
        }

        // --- 2. SPECULAR MATERIALS (Mirror, Conductor, Dielectric) ---
        if (mat.type == MaterialType::Mirror || mat.type == MaterialType::Conductor || mat.type == MaterialType::Dielectric) {
            auto samples = mat.sample_f(wo, rec, sampler.get_2d(pid, sid, depth));
            if (samples.empty()) break;

            BxDFSample bs;
            if (mat.type == MaterialType::Dielectric && samples.size() > 1) {
                // STOCHASTIC SELECTION: Pick Reflection or Refraction
                float xi = sampler.get_1d(pid, sid, depth * 100); // Unique dimension
                float Fr = samples[0].weight.r; // Fresnel reflection term
                
                if (xi < Fr) {
                    bs = samples[0];
                    bs.weight = Color(1.0f); // PDF weight correction: Fr / Fr
                } else {
                    bs = samples[1];
                    bs.weight = Color(1.0f); // PDF weight correction: (1-Fr) / (1-Fr)
                }
            } else {
                bs = samples[0];
            }

            if (!bs.is_valid) break;

            // Beer's Law (Internal Absorption)
            // If the ray is exiting or reflecting while inside (wo dot normal < 0)
            if (mat.type == MaterialType::Dielectric && glm::dot(wo, rec.normal) < 0.0f) {
                throughput *= glm::exp(-mat.absorption_coefficient * rec.t);
            }

            throughput *= bs.weight;
            
            // Correct offset direction for refraction
            // If it's transmission, we move the ray to the OTHER side of the hit point
            float eps = scene.render_context_.intersection_eps;
            Vec3 offset_p = rec.p + (bs.is_transmission ? -rec.normal : rec.normal) * eps;
            
            ray = Ray(offset_p, bs.wi);
            last_was_specular = true;
            continue; 
        }

        // --- 3. NEXT EVENT ESTIMATION ---
        if (config_.use_nee) {
            for (const auto& light : scene.point_lights_) {
                LightSample ls = light->sample_li(rec, sampler.get_2d(pid, sid, depth * 3));
                Ray shadow(rec.p + rec.normal * 1e-4f, ls.wi);
                shadow.t_max = ls.dist - 1e-4f;
                if (!scene.is_occluded(shadow)) {
                    Color f = mat.brdf_cfg.has_value() ? mat.compute_brdf(ls.wi, wo, rec) : mat.evaluate(ls.wi, wo, rec);
                    L += throughput * f * ls.L * std::max(0.0f, glm::dot(rec.normal, ls.wi));
                }
            }
        }

        // --- 4. RUSSIAN ROULETTE ---
        if (config_.use_rr && depth >= 4) {
            float plive = std::min(0.99f, std::max({throughput.r, throughput.g, throughput.b}));
            if (sampler.get_1d(pid, sid, depth * 7) > plive) break;
            throughput /= plive;
        }

        // --- 5. INDIRECT SAMPLING (BRDF) ---
        Frame frame(rec.normal);
        Vec2 u = sampler.get_2d(pid, sid, depth * 11);
        Vec3 wi_local;
        float pdf;

        if (config_.use_importance) {
            float phi = 2.0f * glm::pi<float>() * u.x;
            float r = std::sqrt(u.y);
            wi_local = Vec3(r * std::cos(phi), r * std::sin(phi), std::sqrt(1.0f - u.y));
            pdf = wi_local.z / glm::pi<float>();
        } else {
            float phi = 2.0f * glm::pi<float>() * u.x;
            float cos_theta = u.y;
            float sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);
            wi_local = Vec3(sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta);
            pdf = 1.0f / (2.0f * glm::pi<float>());
        }

        Vec3 wi_world = frame.to_world(wi_local);
        Color f = mat.brdf_cfg.has_value() ? mat.compute_brdf(wi_world, wo, rec) : mat.evaluate(wi_world, wo, rec);
        
        float cos_theta = std::max(0.0f, glm::dot(rec.normal, wi_world));
        if (pdf <= 1e-7f) break;

        throughput *= (f * cos_theta) / pdf;
        ray = Ray(rec.p + rec.normal * 1e-4f, wi_world);
        last_was_specular = false;
        last_brdf_pdf = pdf;
    }

    return L;
}

} // namespace hasmet