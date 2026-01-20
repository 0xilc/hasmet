#pragma once

#include "bxdf.h"
#include "core/types.h"
#include "glm/exponential.hpp"
#include "glm/geometric.hpp"
#include "glm/glm.hpp"
#include <cmath>
#include <glm/gtc/constants.hpp>
#include "core/frame.h"
#include "material/material.h"

namespace hasmet {

namespace {
inline float fresnel_dielectric(float cosThetaI, float etaI, float etaT,
                                float cosThetaT) {
  float r_s = (etaT * cosThetaI - etaI * cosThetaT) /
              (etaT * cosThetaI + etaI * cosThetaT);

  float r_p = (etaI * cosThetaI - etaT * cosThetaT) /
              (etaI * cosThetaI + etaT * cosThetaT);

  return 0.5f * (r_s * r_s + r_p * r_p);
}

inline float fresnel_conductor(float cos_theta, float n, float k) {
  float r_s = ((n * n + k * k) - 2 * n * cos_theta + cos_theta * cos_theta) /
              ((n * n + k * k) + 2 * n * cos_theta + cos_theta * cos_theta);

  float r_p = ((n * n + k * k) * cos_theta * cos_theta - 2 * n * cos_theta + 1) /
              ((n * n + k * k) * cos_theta * cos_theta + 2 * n * cos_theta + 1);

  return 0.5 * (r_s + r_p);
}

inline float smith_geometry(const Vec3& n, const Vec3& h, const Vec3& wo, const Vec3& wi) {
  float nh = glm::dot(n, h);
  float nwo = glm::dot(n, wo);
  float nwi = glm::dot(n, wi);
  float hwo = glm::dot(h, wo);

  return std::min(1.0f, std::min(2.0f * nh * nwo / hwo, 2.0f * nh * nwi / hwo)); 
}

} // namespace

class LambertianReflection : public BxDF {
public:
  LambertianReflection(const Color& reflectance, bool is_normalized = false)
      : BxDF(BxDFType(BSDF_DIFFUSE | BSDF_REFLECTION)), R(reflectance), is_normalized(is_normalized) {}

  Color f(const Vec3& wo, const Vec3& wi) const override {
    if(is_normalized) {
      return R * glm::one_over_pi<float>();
    }
    return R;
  }

  BxDFSample sample_f(const Vec3& wo, const Vec2& u) const override{
    BxDFSample s;
    float phi = 2.0f * glm::pi<float>() * u.x;
    float r = std::sqrt(u.y);
    s.wi = Vec3(r * std::cos(phi), r * std::sin(phi),
                std::sqrt(std::max(0.0f, 1.0f - u.y)));
    s.pdf = pdf(wo, s.wi);
    s.f = f(wo, s.wi);
    s.sampled_type = type;
    return s;
  }

  float pdf(const Vec3& wo, const Vec3& wi) const override {
    return (wi.z > 0) ? wi.z * glm::one_over_pi<float>() : 0.0f;
  }

private:
  Color R;
  bool is_normalized;
};

class SpecularReflection : public BxDF {
public:
  SpecularReflection(const Color& R)
      : BxDF(BxDFType(BSDF_SPECULAR | BSDF_REFLECTION)), R(R) {}

  Color f(const Vec3& wo, const Vec3& wi) const override {return Color(0.0f);}

  BxDFSample sample_f(const Vec3& wo, const Vec2& u) const override {
    BxDFSample s;
    s.wi = Vec3(-wo.x, -wo.y, wo.z); // Perfect reflection
    s.pdf = 1.0f;
    s.f = R / std::max(1e-6f, std::abs(s.wi.z));
    s.sampled_type = type;
    return s;
  }

  float pdf(const Vec3& wo, const Vec3& wi) const override { return 0.0f; }

private:
  Color R;
};

class DielectricReflection : public BxDF {
public:
  DielectricReflection(float ior)
      : BxDF(BxDFType(BSDF_SPECULAR | BSDF_REFLECTION)), ior(ior) {}
  Color f(const Vec3& wo, const Vec3& wi) const override {return Color(0.0f);}

  BxDFSample sample_f(const Vec3& wo, const Vec2& u) const override {
    BxDFSample s;
    s.wi = Vec3(-wo.x, -wo.y, wo.z);
    s.pdf = 1.0f;

    bool entering = wo.z > 0;
    float etaI = entering ? 1.0f : ior;
    float etaT = entering ? ior : 1.0;

    float cosThetaI = std::abs(wo.z);
    float sin2ThetaI = std::max(0.0f, 1.0f - cosThetaI * cosThetaI);
    float sin2ThetaT = (etaI / etaT) * (etaI / etaT) * sin2ThetaI;
    
    float F = 1.0f;
    if (sin2ThetaT < 1.0f) {
      float cosThetaT = std::sqrt(1.0f - sin2ThetaT);
      F = fresnel_dielectric(cosThetaI, etaI, etaT, cosThetaT);
    }

    s.f = Color(F) / std::max(1e-6f, std::abs(s.wi.z));
    s.sampled_type = type;
    return s;
  }

  float pdf(const Vec3& wo, const Vec3& wi) const override { return 0.0f; }

private:
  float ior;
};

class SpecularTransmission : public BxDF {
public:
  SpecularTransmission(const Color& T, float ior)
      : BxDF(BxDFType(BSDF_SPECULAR | BSDF_TRANSMISSION)), T(T), ior(ior) {}

  Color f(const Vec3& wo, const Vec3& wi) const override { return Color(0.0f); }

  BxDFSample sample_f(const Vec3& wo, const Vec2& u) const override {
    BxDFSample s;
    bool entering = wo.z > 0;
    float etaI = entering ? 1.0f : ior;
    float etaT = entering ? ior : 1.0f;
    float eta = etaI / etaT;

    Vec3 n = Vec3(0, 0, entering ? 1 : -1);
    s.wi = glm::refract(glm::normalize(-wo), n, eta);

    if (glm::length(s.wi) < 0.00001f) {
      return {};
    }

    s.pdf = 1.0f;
    float cos_thetaI = std::abs(wo.z);
    float cos_thetaT = std::abs(s.wi.z);

    float F = fresnel_dielectric(cos_thetaI, etaI, etaT, cos_thetaT);

    s.f = T * (1.0f - F) / std::max(1e-6f, std::abs(s.wi.z));
    s.sampled_type = type;

    return s;
  }

  float pdf(const Vec3& wo, const Vec3& wi) const override { return 0.0f; }

private:
  Color T;        // Trasmittance color
  float ior;      // index of refraction
};

class ConductorReflection : public BxDF {
public:
  ConductorReflection(const Color& eta, const Color& k)
      : BxDF(BxDFType(BSDF_SPECULAR | BSDF_REFLECTION)), eta(eta), k(k) {}

  Color f(const Vec3& wo, const Vec3& wi) const override { return Color(0.0f); }

  BxDFSample sample_f(const Vec3& wo, const Vec2& u) const override {
    BxDFSample s;
    s.wi = Vec3(-wo.x, -wo.y, wo.z);
    s.pdf = 1.0f;

    float cos_theta = std::abs(s.wi.z);
    Color F;
    F.r = fresnel_conductor(cos_theta, eta.r, k.r);
    F.g = fresnel_conductor(cos_theta, eta.g, k.g);
    F.b = fresnel_conductor(cos_theta, eta.b, k.b);

    s.f = F / std::max(1e-6f, std::abs(s.wi.z));
    s.sampled_type = type;

    return s;
  }

  float pdf(const Vec3& wo, const Vec3& wi) const override { return 0.0f; }

private:
  Color eta;
  Color k;
};

class MicrofacetReflection : public BxDF {
public:
  MicrofacetReflection(const Color& ks, float p, float ior)
      : BxDF(BxDFType(BSDF_GLOSSY | BSDF_REFLECTION)), ks(ks), p(p), ior(ior) {}

  Color f(const Vec3& wo, const Vec3& wi) const override {
    if (wo.z <= 0 || wi.z <= 0) return Color(0.0f);

    Vec3 h = glm::normalize(wo + wi);
    float cos_theta_h = std::max(0.0f, h.z);
    float cos_theta_i = std::max(0.0f, wi.z);
    float cos_theta_o = std::max(0.0f, wo.z);

    float D = ((p + 2.0f) / (2.0f * glm::pi<float>())) * std::pow(cos_theta_h, p);
    
    float G = smith_geometry(Vec3(0, 0, 1), h, wo, wi);

    float cos_beta = std::max(0.0f, glm::dot(h, wi));
    float R0 = std::pow((1.0f - ior) / (1.0f + ior), 2.0f);
    float F = R0 + (1.0f - R0) * std::pow(1.0f - cos_beta, 5.0f);

    return ks * (D * G * F) / (4.0f * std::max(0.0001f,cos_theta_i * cos_theta_o));
  }

  BxDFSample sample_f(const Vec3& wo, const Vec2& u) const override {
    BxDFSample s;
    if (wo.z <= 0) return s;

    float phi = 2.0f * glm::pi<float>() * u.x;
    float cos_theta_h = std::pow(u.y, 1.0f / (p + 1.0f));
    float sin_theta_h = std::sqrt(std::max(0.0f, 1.0f - cos_theta_h * cos_theta_h));

    Vec3 h(sin_theta_h * std::cos(phi), sin_theta_h * std::sin(phi), cos_theta_h);
    s.wi = glm::reflect(-wo, h);

    if (s.wi.z <= 0) {
      s.pdf = 0.0f;
      return s;
    }

    s.pdf = pdf(wo, s.wi);
    s.f = f(wo, s.wi);
    s.sampled_type = type;
    return s;
  }

  float pdf(const Vec3& wo, const Vec3& wi) const override {
    if (wo.z <= 0 || wi.z <= 0) return 0.0f;

    Vec3 h = glm::normalize(wo + wi);
    float cos_theta_h = std::max(0.0f, h.z);
    float cos_theta_d = std::max(0.0f, glm::dot(h, wi));

    float pdf_h = ((p + 1.0f) / (2.0f * glm::pi<float>())) * std::pow(cos_theta_h, p);
    return pdf_h / (4.0f * std::max(0.0001f,cos_theta_d));
  }

private:
  Color ks;
  float p;
  float ior;
};

class BlinnPhongReflection : public BxDF {
public:
  BlinnPhongReflection(const Color& ks, float p, bool is_modified, bool is_normalized)
      : BxDF(BxDFType(BSDF_GLOSSY | BSDF_REFLECTION)), ks(ks), p(p), is_modified(is_modified), is_normalized(is_normalized) {}

  Color f(const Vec3& wo, const Vec3& wi) const override {
    if (wo.z <= 0 || wi.z <= 0) return Color(0.0f);

    Vec3 h = glm::normalize(wo + wi);
    float cos_alpha_h = std::max(0.0f, h.z);
    float norm = is_normalized ? (p + 8.0f) / (8.0f * glm::pi<float>()) : 1.0f;
    float denom = is_modified ? 1.0f : std::max(0.001f, wi.z);

    return ks * norm * glm::pow(cos_alpha_h, p) / denom;
  }

  BxDFSample sample_f(const Vec3& wo, const Vec2& u) const override {
    BxDFSample s;
    if (wo.z <= 0) return s;

    float phi = 2.0f * glm::pi<float>() * u.x;
    float cos_theta_h = glm::pow(u.y, 1.0f / (p + 1.0f));
    float sin_theta_h = glm::sqrt(glm::max(0.0f, 1.0f - cos_theta_h * cos_theta_h));

    Vec3 h(sin_theta_h * std::cos(phi), sin_theta_h * std::sin(phi), cos_theta_h);

    s.wi = glm::reflect(-wo, h);

    if (s.wi.z <= 0) {
      s.pdf = 0.0f;
      return s;
    }

    s.pdf = pdf(wo, s.wi);
    s.f = f(wo, s.wi);
    s.sampled_type = type;
    return s;
  }

  float pdf(const Vec3& wo, const Vec3& wi) const override { return 0.0f; }

private:
  Color ks;
  float p;
  bool is_modified;
  bool is_normalized;
};

class PhongReflection : public BxDF {
public:
  PhongReflection(const Color& ks, float p, bool is_modified, bool is_normalized)
      : BxDF(BxDFType(BSDF_GLOSSY | BSDF_REFLECTION)), 
        ks(ks), p(p), is_modified(is_modified), is_normalized(is_normalized) {}

  Color f(const Vec3& wo, const Vec3& wi) const override {
    if (wo.z <= 0 || wi.z <= 0) return Color(0.0f);

    Vec3 r = Vec3(-wi.x, -wi.y, wi.z);
    
    float cos_alpha_r = std::max(0.0f, glm::dot(r, wo));
    float norm = is_normalized ? (p + 2.0f) / (2.0f * glm::pi<float>()) : 1.0f;
    float denom = is_modified ? 1.0f : std::max(0.001f, wi.z);

    return ks * norm * std::pow(cos_alpha_r, p) / denom;
  }

  BxDFSample sample_f(const Vec3& wo, const Vec2& u) const override {
    BxDFSample s;
    if (wo.z <= 0) return s;

    float phi = 2.0f * glm::pi<float>() * u.x;
    float cos_theta = std::pow(u.y, 1.0f / (p + 1.0f));
    float sin_theta = std::sqrt(std::max(0.0f, 1.0f - cos_theta * cos_theta));
    Vec3 local_v(sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta);
    
    Vec3 r_wo = Vec3(-wo.x, -wo.y, wo.z);
    
    Frame reflection_frame(r_wo);
    s.wi = reflection_frame.to_world(local_v);

    if (s.wi.z <= 0) {
      s.pdf = 0.0f;
      return s;
    }

    s.pdf = pdf(wo, s.wi);
    s.f = f(wo, s.wi);
    s.sampled_type = type;
    return s;
  }

  float pdf(const Vec3& wo, const Vec3& wi) const override {
    if (wo.z <= 0 || wi.z <= 0) return 0.0f;

    Vec3 r_wo = Vec3(-wo.x, -wo.y, wo.z);
    float cos_alpha_r = std::max(0.0f, glm::dot(r_wo, wi));
    
    return ((p + 1.0f) / (2.0f * glm::pi<float>())) * std::pow(cos_alpha_r, p);
  }

private:
  Color ks;
  float p;
  bool is_modified;
  bool is_normalized;
};

// TODO: fix this. now it acts as lambertian surface.
class UnlitBxDF : public BxDF {
public:
  UnlitBxDF(const Color& color) : BxDF(BxDFType(BSDF_UNLIT)), color(color) {}

  Color f(const Vec3& wo, const Vec3& wi) const override {
    return color * glm::one_over_pi<float>();
  }

  BxDFSample sample_f(const Vec3& wo, const Vec2& u) const override {
    BxDFSample s;
    float phi = 2.0f * glm::pi<float>() * u.x;
    float r = std::sqrt(u.y);
    s.wi = Vec3(r * std::cos(phi), r * std::sin(phi),
                std::sqrt(std::max(0.0f, 1.0f - u.y)));
    s.pdf = pdf(wo, s.wi);
    s.sampled_type = type;
    return s;
  }

  float pdf(const Vec3& wo, const Vec3& wi) const override {
    return (wi.z > 0) ? wi.z * glm::one_over_pi<float>() : 0.0f;
  }

private:
  Color color;
};

} // namespace hasmet