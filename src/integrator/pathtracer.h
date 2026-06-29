#pragma once
#include "core/hit_record.h"
#include "core/ray.h"
#include "integrator.h"
#include "core/types.h"
#include "core/sampler.h"
#include "integrator/whitted.h"
#include <string>
#include <vector>

namespace hasmet {

struct ShadingContext;

class PathTracerIntegrator : public Integrator {
 public:
  PathTracerIntegrator() = default;

  void configure(const std::vector<std::string>& params);

  virtual void render(const Scene& scene, Film& film,
                      const Camera& camera) const override;

 private:
  Color trace_path(Ray& ray, const Scene& scene, SamplingContext& ctx, int max_depth) const;
  Color estimate_direct(const Scene& scene, const BSDF& bsdf, const HitRecord& rec, const Vec3& woW, SamplingContext& ctx, int depth) const;
  float mis_weight(float pdf_a, float pdf_b) const;
  Sampler sampler_;

  enum class MISHeuristic { Balance, Power, ZeroOne };
  struct RenderConfig {
    bool use_nee = false;
    bool use_importance = false;
    bool use_rr = false;
    bool use_mis = false;
    MISHeuristic mis_heuristic = MISHeuristic::Balance;
  } config_;
};

}