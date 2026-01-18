#pragma once

#include "bxdf.h"
#include <cmath>
#include <glm/gtc/constants.hpp>

namespace hasmet {

class LambertianReflection : public BxDF {
public:
  LambertianReflection(const Color& reflectance)
  : BxDF(BxDFType(BSDF_DIFFUSE | BSDF_REFLECTION)), R(reflectance) {}

  Color f(const Vec3& wo, const Vec3& wi) const override {
    return R * glm::one_over_pi<float>();
  }

  BxDFSample sample_f(const Vec3& wo, const Vec2& u) const override{
    BxDFSample s;
    float phi = 2.0f * glm::pi<float>() * u.x;
    float r = std::sqrt(u.y);
    s.wi = Vec3(r * std::cos(phi), r * std::sin(phi), std::sqrt(std::max(0.0f, 1.0f - u.y)));
    s.pdf = pdf(wo, s.wi);
    s.sampled_type = type;
    return s;
  }

  float pdf(const Vec3& wo, const Vec3& wi) const override {
    return (wi.z > 0) ? wi.z * glm::one_over_pi<float>() : 0.0f;
  }

private:
  Color R;
};

class SpecularReflection : public BxDF {
public:
  SpecularReflection(const Color& R) : BxDF(BxDFType(BSDF_SPECULAR | BSDF_REFLECTION)), R(R){}

  Color f(const Vec3& wo, const Vec3& wi) const override {return Color(0.0f);}

  BxDFSample sample_f(const Vec3& wo, const Vec2& u) const override {
    BxDFSample s;
    s.wi = Vec3(-wo.x, -wo.y, wo.z); // Perfect reflection
    s.pdf = 1.0f;
    s.f = R / std::abs(s.wi.z);
    s.sampled_type = type;
    return s;
  }

  float pdf(const Vec3& wo, const Vec3& wi) const override { return 0.0f; }

private:
  Color R;
};


}