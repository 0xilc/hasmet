#pragma once

#include "bxdf.h"
#include "core/frame.h"
#include "core/hit_record.h"

namespace hasmet {

class BSDF {
public:
  static constexpr int MAX_BXDFS = 4;
  const BxDF *bxdfs[MAX_BXDFS];
  int count = 0;
  Frame frame;

  BSDF(const HitRecord &rec);

  void add(const BxDF *bxdf);
  Color evaluate(const Vec3 &wo_world, const Vec3 &wi_world) const;
  Color sample_f(const Vec3 &wo_world, Vec3 *wi_world, const glm::vec2 &u,
                 float *pdf, BxDFType *sampledType) const;
};
} // namespace hasmet