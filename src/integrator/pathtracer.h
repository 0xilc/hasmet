#pragma once
#include "core/hit_record.h"
#include "core/ray.h"
#include "integrator.h"
#include "core/types.h"
#include "core/sampler.h"
#include "integrator/whitted.h"

namespace hasmet {

struct ShadingContext;

class PathTracerIntegrator : public Integrator {
 public:
  PathTracerIntegrator() = default;

  virtual void render(const Scene& scene, Film& film,
                      const Camera& camera) const override;

 private:
  Color trace_path(Ray& ray, const Scene& scene, SamplingContext& ctx, int max_depth) const;
  Color estimate_direct(const Scene& scene, const BSDF& bsdf, const HitRecord& rec, const Vec3& woW, SamplingContext& ctx, int depth) const;
  Sampler sampler_;
  struct RenderConfig {
    bool use_nee;
    bool use_importance;
    bool use_rr; 
    bool use_mis;
  } config_;
};

}