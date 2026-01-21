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
          SamplingContext ctx{local_sampler, pixel_id, s, camera.num_samples_};
          
          glm::vec2 u_pixel = local_sampler.get_2d(pixel_id, s, 0);
          glm::vec2 u_lens = local_sampler.get_2d(pixel_id, s, 1);

          Ray ray = camera.generateRay(static_cast<float>(x), static_cast<float>(y), u_pixel, u_lens);
          
          pixel_color += trace_path(ray, scene, ctx, 6);
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
    
    for (int depth = 0; depth < max_depth; ++depth) {
        HitRecord rec;
        if (!scene.intersect(ray,rec)) {
          if (scene.environment_light_) {
            L += throughput * scene.environment_light_->sample_le(ray);
          }
          break;
        }

        const Material &mat = *scene.get_material(rec.material_id);
        BSDF bsdf(rec);
        mat.setup_bsdf(rec, bsdf);
        Vec3 woW = -glm::normalize(ray.direction);

        if (rec.radiance.has_value()) {
          Color emission = rec.radiance.value();
          L += throughput * emission;
          // if (is_specular_bounce) {
          //   L += throughput * emission;
          // } else {
          //   float light_pdf = scene.light_pdf(ray, rec);
          //   float brdf_pdf = bsdf.pdf(woW, ray.direction);
          //   float weight = balance_heuristic(brdf_pdf, light_pdf);
          //   L += throughput * emission * weight;
          // }
        }

        if (depth >= max_depth) break;

        // TODO: add nee
        Vec2 u = ctx.sampler.get_2d(ctx.pixel_id, ctx.sample_index, depth + 10);
        BxDFSample bs = bsdf.sample_f(woW, u);

        if (bs.pdf <= 0.0f || glm::length(bs.f) == 0.0f) break;

        float cos_theta = std::abs(glm::dot(bs.wi, rec.normal));
        throughput *= (bs.f * cos_theta) / bs.pdf;

        is_specular_bounce = (bs.sampled_type & BSDF_SPECULAR) != 0;

        if (depth >= 3) {
          float p_live = std::min(std::max({throughput.r, throughput.g, throughput.b}), 0.99f);
          if (ctx.sampler.get_1d(ctx.pixel_id, ctx.sample_index, depth + 50) > p_live) break;

          throughput /= p_live;
        }

        ray = Ray(rec.p + bs.wi * 0.0001f, bs.wi);
    }

    return L;
}

} // namespace hasmet