#pragma once

#include "core/types.h"

namespace hasmet {

enum BxDFType {
    BSDF_DIFFUSE      = 1 << 0,
    BSDF_GLOSSY       = 1 << 1,
    BSDF_SPECULAR     = 1 << 2,
    BSDF_REFLECTION   = 1 << 3,
    BSDF_TRANSMISSION = 1 << 4,
    BSDF_UNLIT        = 1 << 5,
    BSDF_ALL          = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION | BSDF_TRANSMISSION
};

struct BxDFSample {
  Color f = Color(0.0f);
  Vec3 wi = Vec3(0.0f);
  float pdf = 0.0f;
  BxDFType sampled_type;
};

class BxDF {
public: 
  BxDF(BxDFType type) : type(type) {}

  virtual ~BxDF() = default;

  virtual Color f(const Vec3& wo, const Vec3& wi) const = 0;

  virtual BxDFSample sample_f(const Vec3& wo, const Vec2& u) const = 0;

  virtual float pdf(const Vec3& wo, const Vec3& wi) const = 0;

  bool matches_flags(BxDFType t) const { return (type & t) == type; }

  const BxDFType type;
};
}