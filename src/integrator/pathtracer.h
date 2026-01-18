#pragma once
#include "core/ray.h"
#include "integrator.h"
#include "core/types.h"
#include "core/sampler.h"

namespace hasmet {

struct ShadingContext;

class PathTracerIntegrator : public Integrator {
 public:
  PathTracerIntegrator() = default;

  virtual void render(const Scene& scene, Film& film,
                      const Camera& camera) const override;

 private:
  Color trace_path(Ray& ray, const Scene& scene, int max_depth) const;
  Sampler sampler_;
};

}