#include "material.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "core/ray.h"
#include "core/types.h"
#include "glm/exponential.hpp"
#include "glm/geometric.hpp"

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
    if (roughness <= 0.0f) return d;
    Vec3 w = d;
    Vec3 a = (std::abs(w.x) > 0.9f) ? Vec3(0, 1, 0) : Vec3(1, 0, 0);
    Vec3 u_axis = glm::normalize(glm::cross(a, w));
    Vec3 v_axis = glm::cross(w, u_axis);
    float r1 = u_sample.x - 0.5f;
    float r2 = u_sample.y - 0.5f;
    
    return glm::normalize(d + (u_axis * r1 + v_axis * r2) * roughness);
}
} // namespace

Color Material::evaluate(const Vec3& wi, const Vec3& wo, const HitRecord& rec) const {
  if (type == MaterialType::Unlit 
      || type == MaterialType::Dielectric) {
    return Color(0.0f);
  };
  
  Color diffuse = diffuse_reflectance;

  Vec3 h = glm::normalize(wi + wo);
  float cos_alpha = std::max(0.0f, glm::dot(rec.normal, h));
  Color specular = specular_reflectance * glm::pow(cos_alpha, phong_exponent);

  return diffuse + specular;
}

// TODO: convert this to one sample when we move to path tracing
std::vector<BxDFSample> Material::sample_f(const Vec3& wo, const HitRecord& rec, const glm::vec2& u) {
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


} // namespace hasmet

