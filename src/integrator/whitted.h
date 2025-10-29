#pragma once
#include "core/ray.h"
#include "core/color.h"
#include "integrator.h"

class WhittedIntegrator : public Integrator {
 public:
  WhittedIntegrator(int max_depth);

  virtual void render(const Scene& scene, Film& film, const Camera& camera) const override;

 private:
  Color Li(Ray& ray, const Scene& scene, int depth) const;
  int max_depth_;
};