#include "pathtracer.h"
#include "core/frame.h"
#include "core/hit_record.h"
#include "core/timer.h"
#include "core/sampler.h"
#include "film/film.h"
#include "camera/camera.h"
#include "glm/geometric.hpp"
#include "glm/glm.hpp"
#include "material/material.h"
#include "material/material_manager.h"
#include "scene/scene.h"
#include "core/sampler.h"

namespace hasmet {

void PathTracerIntegrator::render(const Scene& scene, Film& film, const Camera& camera) const {
  SCOPED_TIMER("Render: [Path Tracing]");
  int width = film.getWidth();
  int height = film.getHeight();
  int samples_per_pixel = camera.num_samples_;
  int max_depth = scene.render_context_.max_recursion_depth ? scene.render_context_.max_recursion_depth : 6;

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

          pixel_color += trace_path(scene, ray, max_depth);
        }
        film.addSample(x, y, pixel_color / static_cast<float>(camera.num_samples_));
      }
    }
  }
}

Color PathTracerIntegrator::trace_path(const Scene& scene, Ray ray, int max_depth) const {
  Color L(0.0f);
  Color throughput(1.0f);
  Sampler sampler;
  float isect_eps = scene.render_context_.intersection_eps;

  for (int depth = 0; depth < max_depth; ++depth) {
    HitRecord rec;
    if (!scene.intersect(ray, rec)) {
      L += throughput * ((scene.environment_lights_.empty()) ? scene.render_context_.background_color : scene.environment_lights_[0]->sample_le(ray));
      break;
    }

    if (rec.radiance.has_value()) {
      L += throughput * rec.Le(-ray.direction);
    }

    const Material& mat = MaterialManager::get_instance()->get(rec.material_id);
    Frame shading_frame(rec.normal);
    Vec3 wo_world = -glm::normalize(ray.direction);
    Vec3 wo_local = shading_frame.to_local(wo_world);

    Vec2 u = sampler.get_2d(0, 0, depth + 10);
    BxDFSample bs = mat.sample(wo_world, rec, u);
    
    if (!bs.is_valid) break;

    throughput *= bs.weight;

    // Next ray
    Vec3 offset_dir = bs.is_transmission ? -rec.normal : rec.normal;
    ray = Ray(rec.p + offset_dir * isect_eps, bs.wi);

    if (depth > 3) {
      float continuation_prob = std::min(0.95f, glm::max(throughput.r, glm::max(throughput.g, throughput.b)));
      if (sampler.get_1d(0, 0, depth + 20) > continuation_prob) continue;
      throughput /= continuation_prob;
    }
  }

  return L;
}

} // namespace hasmet