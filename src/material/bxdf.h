#pragma once

#include "core/types.h"
#include <glm/glm.hpp>

namespace hasmet {
enum BxDFType {
  Diffuse       = 1 << 0,
  Glossy        = 1 << 1,
  Specular      = 1 << 2,
  Reflection    = 1 << 3,
  Transmission  = 1 << 4,
  All          = Diffuse | Glossy | Specular | Reflection | Transmission
};

class BxDF {
  public:
    BxDF(BxDFType type) : type_(type) {}
    virtual ~BxDF() = default;

    virtual Color f(const Vec3& wo, const Vec3& wi) const = 0;
    virtual Color sample_f(const Vec3& wo, Vec3* wi, const glm::vec2& u, float* pdf) const = 0;
    virtual float pdf(const Vec3& wo, const Vec3& wi) const = 0;

    BxDFType type_;
};
}