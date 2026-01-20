#pragma once

#include "core/ray.h"
#include "integrator.h"
#include "core/types.h"
#include "core/sampler.h"
#include "material/bsdf.h"

namespace hasmet {

struct ShadingContext;

class WhittedIntegrator : public Integrator {
 public:
  WhittedIntegrator() = default;

  virtual void render(const Scene& scene, Film& film,
                      const Camera& camera) const override;

 private:
  Color trace_ray(Ray& ray, const Scene& scene, int depth, Sampler& sampler, int sample_index, int num_samples, int pixel_id) const;
  Color shade_direct(const BSDF& bsdf, const HitRecord& rec, const Vec3& woW, const Scene& scene, Sampler& sampler, int sample_idx, int pid) const;
  Sampler sampler_;
};
} // namespace hasmet