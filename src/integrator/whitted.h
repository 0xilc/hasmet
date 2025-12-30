#pragma once

#include "core/hit_record.h"
#include "core/ray.h"
#include "integrator.h"
#include "core/types.h"
#include "core/sampler.h"
#include "material/material.h"

namespace hasmet {
class WhittedIntegrator : public Integrator {
 public:
  WhittedIntegrator() = default;

  virtual void render(const Scene& scene, Film& film,
                      const Camera& camera) const override;

 private:
  Color Li(Ray& ray, const Scene& scene, int depth, int sample_index, int num_samples) const;
  Color shade_blinn_phong(const Ray& ray, const HitRecord& rec, const Material& mat,
                              const Scene& scene, int depth, int sample_index, int num_samples) const;
  Sampler sampler_;
};
} // namespace hasmet