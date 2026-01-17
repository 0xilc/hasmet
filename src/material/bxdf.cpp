#include "bxdf.h"
#include "core/logging.h"
#include "glm/geometric.hpp"
#include <cmath>
#include <glm/gtc/constants.hpp>
#include "core/sampling.h"

namespace hasmet {

namespace {
const float PI = glm::pi<float>();

inline float fresnel_dielectric(float cosThetaI, float etaI, float etaT) {
    float eta = etaI / etaT;
    float sin2ThetaT = eta * eta * (1.0f - cosThetaI * cosThetaI);
    if (sin2ThetaT >= 1.0f) return 1.0f; // Tam iç yansıma
    
    float cosThetaT = std::sqrt(std::max(0.0f, 1.0f - sin2ThetaT));
    
    float r_s = (etaI * cosThetaI - etaT * cosThetaT) / (etaI * cosThetaI + etaT * cosThetaT);
    float r_p = (etaT * cosThetaI - etaI * cosThetaT) / (etaT * cosThetaI + etaI * cosThetaT);
    
    return 0.5f * (r_s * r_s + r_p * r_p);
}

inline float fresnel_conductor(float cos_theta, float n, float k) {
  float r_s = ((n * n + k * k) - 2 * n * cos_theta + cos_theta * cos_theta) /
              ((n * n + k * k) + 2 * n * cos_theta + cos_theta * cos_theta);
  float r_p =
      ((n * n + k * k) * cos_theta * cos_theta - 2 * n * cos_theta + 1) /
      ((n * n + k * k) * cos_theta * cos_theta + 2 * n * cos_theta + 1);
  return 0.5 * (r_s + r_p);
}

Vec3 perturb(const Vec3& d, float roughness, const glm::vec2& u_sample) {
  Vec3 w = glm::normalize(d);

  Vec3 tmp =
      (std::abs(w.x) > 0.9f) ? Vec3(0.0f, 1.0f, 0.0f) : Vec3(1.0f, 0.0f, 0.0f);
  Vec3 u = glm::normalize(glm::cross(tmp, w));
  Vec3 v = glm::cross(w, u);

  float r1 = Sampling::_generate_random_float(0.0f, 1.0f) - 0.5;
  float r2 = Sampling::_generate_random_float(0.0f, 1.0f) - 0.5;

  Vec3 new_direction = d + (u * r1 + v * r2) * roughness;
  return glm::normalize(new_direction);
}

Vec3 sample_cosine_hemisphere(const glm::vec2& u, float& pdf) {
  float phi = 2.0f * PI * u.x;
  float cosTheta = std::sqrt(u.y);
  float sinTheta = std::sqrt(1.0f - u.y);
  pdf = cosTheta / PI;
  return Vec3(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);
}
} // namespace

Color OriginalPhong::evaluate(const Vec3& wo, const Vec3& wi) const {
    if (wi.z <= 0.0f || wo.z <= 0.0f) return Color(0.0f);
    Vec3 r = glm::reflect(wi, Vec3(0, 0, 1));
    float cosAlpha = std::max(0.0f, glm::dot(r, wo));
    
    return (kd_ / PI) + (ks_ * std::pow(cosAlpha, exponent_));
}

float OriginalPhong::pdf(const Vec3& wo, const Vec3& wi) const {
    return (wi.z > 0.0f) ? wi.z / PI : 0.0f;
}

Color OriginalPhong::sample(const Vec3& wo, Vec3& wi, const glm::vec2& u, float& pdf_val) const {
    wi = sample_cosine_hemisphere(u, pdf_val);
    return evaluate(wo, wi);
}

Color OriginalBlinnPhong::evaluate(const Vec3& wo, const Vec3& wi) const {
    if (wi.z <= 0.0f || wo.z <= 0.0f) return Color(0.0f);
    Vec3 wh = glm::normalize(wi + wo);
    float cosThetaH = std::max(0.0f, wh.z);
    return (kd_ / PI) + (ks_ * std::pow(cosThetaH, exponent_));
}

float OriginalBlinnPhong::pdf(const Vec3& wo, const Vec3& wi) const {
    return (wi.z > 0.0f) ? wi.z / PI : 0.0f;
}

Color OriginalBlinnPhong::sample(const Vec3& wo, Vec3& wi, const glm::vec2& u, float& pdf_val) const {
    wi = sample_cosine_hemisphere(u, pdf_val);
    return evaluate(wo, wi);
}

Color ModifiedPhong::evaluate(const Vec3& wo, const Vec3& wi) const {
    if (wi.z <= 0.0f || wo.z <= 0.0f) return Color(0.0f);
    Vec3 r = reflect(wi, Vec3(0, 0, 1));
    float cosAlpha = std::max(0.0f, glm::dot(r, wo));
    float norm = normalized_ ? (exponent_ + 2.0f) / (2.0f * PI) : 1.0f;
    return (kd_ / PI) + (ks_ * norm * std::pow(cosAlpha, exponent_));
}

float ModifiedPhong::pdf(const Vec3& wo, const Vec3& wi) const {
    return (wi.z > 0.0f) ? wi.z / PI : 0.0f;
}

Color ModifiedPhong::sample(const Vec3& wo, Vec3& wi, const glm::vec2& u, float& pdf_val) const {
    wi = sample_cosine_hemisphere(u, pdf_val);
    return evaluate(wo, wi);
}

// --- Modified Blinn-Phong ---
Color ModifiedBlinnPhong::evaluate(const Vec3& wo, const Vec3& wi) const {
    if (wi.z <= 0.0f || wo.z <= 0.0f) return Color(0.0f);
    Vec3 wh = glm::normalize(wi + wo);
    float cosThetaH = std::max(0.0f, wh.z);
    float norm = normalized_ ? (exponent_ + 8.0f) / (8.0f * PI) : 1.0f;
    return (kd_ / PI) + (ks_ * norm * std::pow(cosThetaH, exponent_));
}

float ModifiedBlinnPhong::pdf(const Vec3& wo, const Vec3& wi) const {
    return (wi.z > 0.0f) ? wi.z / PI : 0.0f;
}

Color ModifiedBlinnPhong::sample(const Vec3& wo, Vec3& wi, const glm::vec2& u, float& pdf_val) const {
    wi = sample_cosine_hemisphere(u, pdf_val);
    return evaluate(wo, wi);
}

// --- Torrance-Sparrow ---
Color TorranceSparrowBRDF::evaluate(const Vec3& wo, const Vec3& wi) const {
    if (wi.z <= 0.0f || wo.z <= 0.0f) return Color(0.0f);

    Vec3 wh = glm::normalize(wi + wo);
    float cosThetaH = std::max(0.0f, wh.z);
    float cosThetaO = wo.z;
    float cosThetaI = wi.z;
    float cosThetaD = std::max(0.0f, glm::dot(wi, wh));

    float D = (exponent_ + 2.0f) / (2.0f * PI) * std::pow(cosThetaH, exponent_);
    float F = fresnel_dielectric(cosThetaD, 1.0f, ior_);
    float G = std::min({1.0f, (2.0f * cosThetaH * cosThetaO) / cosThetaD, (2.0f * cosThetaH * cosThetaI) / cosThetaD});

    Color diffuse = kd_ / PI;
    if (kdfresnel_) {
        diffuse *= (1.0f - F);
    }

    Color specular = ks_ * (D * G * F) / (4.0f * cosThetaI * cosThetaO);
    return diffuse + specular;
}

float TorranceSparrowBRDF::pdf(const Vec3& wo, const Vec3& wi) const {
    return (wi.z > 0.0f) ? wi.z / PI : 0.0f;
}

Color TorranceSparrowBRDF::sample(const Vec3& wo, Vec3& wi, const glm::vec2& u, float& pdf_val) const {
    wi = sample_cosine_hemisphere(u, pdf_val);
    return evaluate(wo, wi);
}

// --- Mirror ---
Color MirrorBRDF::sample(const Vec3& wo, Vec3& wi, const glm::vec2& u, float& pdf_val) const {
    wi = Vec3(-wo.x, -wo.y, wo.z);
    pdf_val = 1.0f;
    return refl_ / std::abs(wi.z);
}

// --- Dielectric ---
Color DielectricBSDF::sample(const Vec3& wo, Vec3& wi, const glm::vec2& u, float& pdf_val) const {
    float etaI = 1.0f;
    float etaT = ior_;
    Vec3 n(0, 0, 1);
    float cosThetaI = wo.z;

    if (cosThetaI < 0.0f) {
        std::swap(etaI, etaT);
        n = Vec3(0, 0, -1);
        cosThetaI = -cosThetaI;
    }

    float Fr = fresnel_dielectric(cosThetaI, etaI, etaT);
    pdf_val = 1.0f;

    if (u.x < Fr) {
        wi = reflect(wo, n);
        return Color(Fr) / std::abs(wi.z);
    } else {
        float eta = etaI / etaT;
        float sin2ThetaT = eta * eta * (1.0f - cosThetaI * cosThetaI);
        float cosThetaT = std::sqrt(std::max(0.0f, 1.0f - sin2ThetaT));
        wi = eta * -wo + (eta * cosThetaI - cosThetaT) * n;
        return Color(1.0f - Fr) / std::abs(wi.z);
    }
}
} // namespace hasmet