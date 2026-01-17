#pragma once

#include "integrator.h"
#include "core/types.h"
#include "core/ray.h"

namespace hasmet {

struct ShadingContext;

class PathTracerIntegrator : public Integrator {
 public:
  PathTracerIntegrator() = default;

  virtual void render(const Scene& scene, Film& film,
                      const Camera& camera) const override;

 private:
    Color trace_path(const Scene& scene, Ray ray, int max_depth) const;
  
};
} // namespace hasmet