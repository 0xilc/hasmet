#pragma once
#include "core/color.h"
#include "core/hit_record.h"
#include "core/ray.h"
#include "integrator.h"

class WhittedIntegrator : public Integrator {
 public:
  WhittedIntegrator(int max_depth);

  virtual void render(const Scene& scene, Film& film,
                      const Camera& camera) const override;

 private:
  Color Li(Ray& ray, const Scene& scene, int depth) const;
  Color calculate_blinn_phong(const Ray& ray, const HitRecord& rec,
                              const Scene& scene, int depth) const;
  int max_depth_;
};