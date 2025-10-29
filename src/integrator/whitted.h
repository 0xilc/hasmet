#pragma once
#include "core/ray.h"
#include "core/color.h"
#include "core/hit_record.h"
#include "integrator.h"

class WhittedIntegrator : public Integrator {
 public:
  WhittedIntegrator(int max_depth);

  virtual void render(const Scene& scene, Film& film, const Camera& camera) const override;

 private:
  Color Li(Ray& ray, const Scene& scene, int depth) const;
  Color calculate_blinn_phong(const HitRecord& rec, const Scene& scene,
                              const glm::vec3& view_dir) const;
  int max_depth_;
};