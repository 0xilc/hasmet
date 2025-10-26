#pragma once
#include "core/ray.h"
#include "core/spectrum.h"
#include "integrator.h"

class WhittedIntegrator : public Integrator {
 public:
  WhittedIntegrator(int max_depth);

  virtual void render(const Scene& scene, Film& film) const override;

 private:
  Spectrum Li(const Ray& ray, const Scene& scene, int depth) const;
  int max_depth_;
};