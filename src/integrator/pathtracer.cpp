#include "pathtracer.h"
#include "core/hit_record.h"
#include "core/sampling.h"
#include "core/timer.h"
#include "core/types.h"
#include "film/film.h"
#include "material/material_manager.h"
#include "scene/scene.h"
#include <cmath>

namespace hasmet {

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

          pixel_color += trace_path(ray, scene, max_depth);
        }
        film.addSample(x, y,
                       pixel_color / static_cast<float>(camera.num_samples_));
      }
    }
  }
}

Color PathTracerIntegrator::trace_path(Ray &ray, const Scene &scene,
                                       int max_depth) const {
  Color L(0.0f);
  Color throughput(1.0f);
  bool last_was_specular = true;

  for (int depth = 0; depth < max_depth; ++depth) {
    HitRecord rec;
    if (!scene.intersect(ray, rec)) {
      L += throughput * ((scene.environment_lights_.empty())
                             ? scene.render_context_.background_color
                             : scene.environment_lights_[0]->sample_le(ray));
      break;
    }

    if (rec.radiance.has_value()) {
      L += throughput * rec.radiance.value();
      break;
    }

    const Material &mat = MaterialManager::get_instance()->get(rec.material_id);
    Vec3 wo = -glm::normalize(ray.direction);

    // TODO: NEE
    
    Vec2 u{Sampling::_generate_random_float(0, 1.0f),
           Sampling::_generate_random_float(0, 1.0f)};

    Vec3 wi_local;
    float pdf;
    if (true) { // use_importance sampling
      float phi = 2.0f * glm::pi<float>() * u.x;
      float sin_theta = std::sqrt(u.y);
      wi_local = Vec3(std::cos(phi) * sin_theta, std::sin(phi) * sin_theta,
                      std::sqrt(1.0f - u.y));
      pdf = wi_local.z / glm::pi<float>();
    } else {
      // uniform sampling
    }

    Vec3 wi_world = local_to_world(wi_local, rec.normal);

    Color f = mat.diffuse_reflectance / glm::pi<float>();

    float cos_theta = std::max(0.0f, glm::dot(rec.normal, wi_world));
    if (pdf <= 0.0f) break;

    throughput *= (f * cos_theta) / pdf;

    ray = Ray(rec.p + rec.normal * 1e-4f, wi_world);
    last_was_specular = false;
  }

  return L;
}

} // namespace hasmet