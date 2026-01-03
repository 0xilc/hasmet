#pragma once

#include "core/ray.h"
#include "integrator.h"
#include "core/types.h"
#include "core/sampler.h"

namespace hasmet {

struct ShadingContext;

class WhittedIntegrator : public Integrator {
 public:
  WhittedIntegrator() = default;

  virtual void render(const Scene& scene, Film& film,
                      const Camera& camera) const override;

 private:
  Color Li(Ray& ray, const Scene& scene, int depth, Sampler& sampler, int sample_index, int num_samples, int pixel_id) const;
  Color shade_blinn_phong(const ShadingContext& ctx, const Scene& scene) const;
  Sampler sampler_;
};
} // namespace hasmet