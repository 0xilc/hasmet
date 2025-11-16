#pragma once
#include "core/color.h"
#include "core/hit_record.h"
#include "core/ray.h"
#include "integrator.h"

class Normals : public Integrator {
 public:
  Normals(int max_depth);

  virtual void render(const Scene& scene, Film& film,
                      const Camera& camera) const override;

 private:
  Color Li(Ray& ray, const Scene& scene, int depth) const;
  int max_depth_;
};