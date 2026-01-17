#include "material.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "core/ray.h"
#include "core/sampling.h"
#include "core/types.h"
#include "glm/exponential.hpp"
#include "glm/geometric.hpp"
#include "core/frame.h"

namespace hasmet {
namespace {
inline float fresnel_dielectric(float cosThetaI, float etaI, float etaT, float cosThetaT) {
  float r_s = (etaT * cosThetaI - etaI * cosThetaT) /

              (etaT * cosThetaI + etaI * cosThetaT);

  float r_p = (etaI * cosThetaI - etaT * cosThetaT) /

              (etaI * cosThetaI + etaT * cosThetaT);

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
} // namespace

Color Material::evaluate(const Vec3& wi, const Vec3& wo, const HitRecord& rec) const {
  if (type == MaterialType::Unlit 
      || type == MaterialType::Dielectric) {
    return Color(0.0f);
  };
  
  Color diffuse = diffuse_reflectance;

  // INFO: dividing specular to cos_theta is physically wrong, 
  // but I put it here to mimic the old algorithm
  Vec3 h = glm::normalize(wi + wo);
  float cos_theta = std::max(0.001f, glm::dot(rec.normal, wi));
  float cos_alpha = std::max(0.0f, glm::dot(rec.normal, h));
  Color specular = specular_reflectance * glm::pow(cos_alpha, phong_exponent) / cos_theta;

  return diffuse + specular;
}

// TODO: convert this to one sample when we move to path tracing
std::vector<BxDFSample> Material::sample_f(const Vec3& wo, const HitRecord& rec, const glm::vec2& u) const{
  std::vector<BxDFSample> samples;

  if (type == MaterialType::Mirror) {
    BxDFSample s;
    s.wi = perturb(glm::reflect(-wo, rec.normal), roughness, u);
    s.weight = mirror_reflectance;
    s.is_valid = true;
    samples.push_back(s);
  }
  else if (type == MaterialType::Conductor) {
    BxDFSample s;
    s.wi = perturb(glm::reflect(-wo, rec.normal), roughness, u);
    float cos_theta = std::max(0.0f, glm::dot(rec.normal, s.wi));
    float Fr = fresnel_conductor(cos_theta, refraction_index, absorption_index);
    s.weight = Fr * mirror_reflectance;
    s.is_valid = true;
    samples.push_back(s);
  }
  else if (type == MaterialType::Dielectric) {
    bool entering = glm::dot(wo, rec.normal) > 0.0f;
    Vec3 n = entering ? rec.normal : -rec.normal;
    float etaI = entering ? 1.0f : refraction_index;
    float etaT = entering ? refraction_index : 1.0f;
    float eta = etaI / etaT;

    float cosThetaI = glm::dot(wo, n);
    float sin2ThetaT = eta * eta * (1.0f - cosThetaI * cosThetaI);

    BxDFSample refl;
    refl.wi = perturb(glm::reflect(-wo, n), roughness, u);
    refl.is_transmission = false;
    refl.is_valid = true;
    if (sin2ThetaT > 1.0f) { // Total internal reflection
      refl.weight = Color(1.0f);
      samples.push_back(refl);
    } else {
      float cosThetaT = glm::sqrt(1.0f - sin2ThetaT);
      float Fr = fresnel_dielectric(cosThetaI, etaI, etaT, cosThetaT);
      refl.weight = Color(Fr);
      samples.push_back(refl);

      // Refraction ray
      BxDFSample refr;
      refr.wi = perturb(eta * -wo + (eta * cosThetaI - cosThetaT) * n, roughness, u);
      refr.weight = Color(1.0f - Fr);
      refr.is_transmission = true;
      refr.is_valid = true;
      samples.push_back(refr);
    }
  }
  return samples;
}

BxDFSample Material::sample(const Vec3& wo, const HitRecord& rec, const glm::vec2& u) const{
  BxDFSample s;

  if (type == MaterialType::Mirror) {
    s.wi = perturb(glm::reflect(-wo, rec.normal), roughness, u);
    s.weight = mirror_reflectance;
    s.is_valid = true;
  }
  else if (type == MaterialType::Conductor) {
    s.wi = perturb(glm::reflect(-wo, rec.normal), roughness, u);
    float cos_theta = std::max(0.0f, glm::dot(rec.normal, s.wi));
    float Fr = fresnel_conductor(cos_theta, refraction_index, absorption_index);
    s.weight = Fr * mirror_reflectance;
    s.is_valid = true;
  }
  else if (type == MaterialType::Dielectric) {
    bool entering = glm::dot(wo, rec.normal) > 0.0f;
    Vec3 n = entering ? rec.normal : -rec.normal;
    float etaI = entering ? 1.0f : refraction_index;
    float etaT = entering ? refraction_index : 1.0f;
    float eta = etaI / etaT;

    float cosThetaI = glm::dot(wo, n);
    float sin2ThetaT = eta * eta * (1.0f - cosThetaI * cosThetaI);

    BxDFSample refl;
    refl.wi = perturb(glm::reflect(-wo, n), roughness, u);
    refl.is_transmission = false;
    refl.is_valid = true;

    if (sin2ThetaT > 1.0f) { // Total internal reflection
      refl.weight = Color(1.0f);
      s = refl;
    }
    else {
      float cosThetaT = glm::sqrt(1.0f - sin2ThetaT);
      float Fr = fresnel_dielectric(cosThetaI, etaI, etaT, cosThetaT);
      refl.weight = Color(Fr);

      // Refraction ray
      BxDFSample refr;
      refr.wi = perturb(eta * -wo + (eta * cosThetaI - cosThetaT) * n, roughness, u);
      refr.weight = Color(1.0f - Fr);
      refr.is_transmission = true;
      refr.is_valid = true;

      float random = Sampling::_generate_random_float(0.0f, 1.0f);
      if (random < Fr) {
        s = refl;
      }
      else {
        s = refr;
      }
    }
  }
  else {
    Frame frame(rec.normal);
    float phi = 2.0f * glm::pi<float>() * u.x;
    float cosTheta = std::sqrt(u.y);
    float sinTheta = std::sqrt(std::max(0.0f, 1.0f - u.y));
    
    Vec3 local_wi(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);
    s.wi = frame.to_world(local_wi);
    
    float cos_theta_i = glm::dot(rec.normal, s.wi);
    if (cos_theta_i <= 1e-6f) { // Prevent division by zero/NaNs
        s.is_valid = false;
        return s;
    }

    Color tmp;
    if (brdf) {
      s.weight = brdf->evaluate(s.wi, wo, rec.normal, diffuse_reflectance, specular_reflectance);
    } else {
      s.weight = diffuse_reflectance / glm::pi<float>();
    }
    s.is_valid = true;
  }

  return s;
}

Color BRDF::evaluate(const Vec3& wi, const Vec3& wo, const Vec3& n, const Color& kd, const Color& ks) {
float cosThetaI = std::max(0.0f, glm::dot(n, wi));
  if (cosThetaI <= 0.0f) return glm::vec3(0.0f);

  glm::vec3 diffuse(0.0f);
  glm::vec3 specular(0.0f);

  glm::vec3 wh = glm::normalize(wi + wo);
  float cosAlphaH = std::max(0.0f, glm::dot(n, wh));

  switch (type)
  {
  case BRDFType::OriginalPhong:
  {
      glm::vec3 r = glm::normalize(2.0f * glm::dot(n, wi) * n - wi);
      float cosAlphaR = std::max(0.0f, glm::dot(r, wo));

      diffuse = kd;
      float spec_factor = std::pow(cosAlphaR, exponent);
      if (cosThetaI > 1e-6f) spec_factor /= cosThetaI;

      specular = ks * spec_factor;
      break;
  }
  case BRDFType::ModifiedPhong:
  {
      glm::vec3 r = glm::normalize(2.0f * glm::dot(n, wi) * n - wi);
      float cosAlphaR = std::max(0.0f, glm::dot(r, wo));

      if (normalized) {
          // Eq 5: kd/PI + ks * (p+2)/2PI * cos^n(alpha_r)
          diffuse = kd * (float)(1.0 / M_PI);
          float norm_factor = (exponent + 2.0f) / (2.0f * M_PI);
          specular = ks * (norm_factor * std::pow(cosAlphaR, exponent));
      } else {
          // Eq 3
          diffuse = kd;
          specular = ks * std::pow(cosAlphaR, exponent);
      }
      break;
  }
  case BRDFType::OriginalBlinnPhong:
  {
      // Eq 7: kd + ks * (cos^p(alpha_h) / cosThetaI)
      diffuse = kd;
      float spec_factor = std::pow(cosAlphaH, exponent);
      if (cosThetaI > 1e-6f) spec_factor /= cosThetaI;

      specular = ks * spec_factor;
      break;
  }
  case BRDFType::ModifiedBlinnPhong:
  {
      if (normalized) {
          // Eq 9: kd/PI + ks * (p+8)/8PI * cos^n(alpha_h)
          diffuse = kd * (float)(1.0 / M_PI);
          float norm_factor = (exponent + 8.0f) / (8.0f * M_PI);
          specular = ks * (norm_factor * std::pow(cosAlphaH, exponent));
      } else {
          // Eq 8: kd + ks * cos^p(alpha_h)
          diffuse = kd;
          specular = ks * std::pow(cosAlphaH, exponent);
      }
      break;
  }
}
return diffuse + specular;
}
} // namespace hasmet

