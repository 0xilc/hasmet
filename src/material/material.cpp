#include "material.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include "core/hit_record.h"
#include "core/ray.h"
#include "core/types.h"
#include "glm/exponential.hpp"
#include "glm/geometric.hpp"
#include "bxdf_library.h"
#include "material/bsdf.h"

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

inline float smith_geometry(const Vec3& n, const Vec3& h, const Vec3& wo, const Vec3& wi) {
  float nh = glm::dot(n, h);
  float nwo = glm::dot(n, wo);
  float nwi = glm::dot(n, wi);
  float hwo = glm::dot(h, wo);

  return std::min(1.0f, std::min(2.0f * nh * nwo / hwo, 2.0f * nh * nwi / hwo));
}
} // namespace

// TODO : fix specular reflection as it must have shininess coefficient too.
void BlinnPhongMaterial::setup_bsdf(HitRecord& rec, BSDF& bsdf) const {
  bsdf.add(new LambertianReflection(kd_));
  bsdf.add(new SpecularReflection(ks_));
}

void MirrorMaterial::setup_bsdf(HitRecord& rec, BSDF& bsdf) const {
  if (roughness_ > 0.0f) {
    // TODO: ADD Microfacet reflection BxDF here.
  }

  bsdf.add(new SpecularReflection(R_));
}

void ConductorMaterial::setup_bsdf(HitRecord& rec, BSDF& bsdf) const {
}

void DielectricMaterial::setup_bsdf(HitRecord& rec, BSDF& bsdf) const {
}

void UnlitMaterial::setup_bsdf(HitRecord& rec, BSDF& bsdf) const {
  // TODO: DO texture lookup and evaluate the color and pass it.
  bsdf.add(new UnlitBxDF(color_));
}

// Color Material::evaluate(const Vec3& wi, const Vec3& wo, const HitRecord& rec) const {
//   if (type == MaterialType::Unlit 
//       || type == MaterialType::Dielectric) {
//     return Color(0.0f);
//   };
  
//   Color diffuse = diffuse_reflectance;

//   // INFO: dividing specular to cos_theta is physically wrong, 
//   // but I put it here to mimic the old algorithm
//   Vec3 h = glm::normalize(wi + wo);
//   float cos_theta = std::max(0.001f, glm::dot(rec.normal, wi));
//   float cos_alpha = std::max(0.0f, glm::dot(rec.normal, h));
//   Color specular = specular_reflectance * glm::pow(cos_alpha, phong_exponent) / cos_theta;

//   return diffuse + specular;
// }

// // TODO: convert this to one sample when we move to path tracing
// std::vector<BxDFSample> Material::sample_f(const Vec3& wo, const HitRecord& rec, const glm::vec2& u) {
//   std::vector<BxDFSample> samples;

//   if (type == MaterialType::Mirror) {
//     BxDFSample s;
//     s.wi = perturb(glm::reflect(-wo, rec.normal), roughness, u);
//     s.weight = mirror_reflectance;
//     s.is_valid = true;
//     samples.push_back(s);
//   }
//   else if (type == MaterialType::Conductor) {
//     BxDFSample s;
//     s.wi = perturb(glm::reflect(-wo, rec.normal), roughness, u);
//     float cos_theta = std::max(0.0f, glm::dot(rec.normal, s.wi));
//     float Fr = fresnel_conductor(cos_theta, refraction_index, absorption_index);
//     s.weight = Fr * mirror_reflectance;
//     s.is_valid = true;
//     samples.push_back(s);
//   }
//   else if (type == MaterialType::Dielectric) {
//     bool entering = glm::dot(wo, rec.normal) > 0.0f;
//     Vec3 n = entering ? rec.normal : -rec.normal;
//     float etaI = entering ? 1.0f : refraction_index;
//     float etaT = entering ? refraction_index : 1.0f;
//     float eta = etaI / etaT;

//     float cosThetaI = glm::dot(wo, n);
//     float sin2ThetaT = eta * eta * (1.0f - cosThetaI * cosThetaI);

//     BxDFSample refl;
//     refl.wi = perturb(glm::reflect(-wo, n), roughness, u);
//     refl.is_transmission = false;
//     refl.is_valid = true;
//     if (sin2ThetaT > 1.0f) { // Total internal reflection
//       refl.weight = Color(1.0f);
//       samples.push_back(refl);
//     } else {
//       float cosThetaT = glm::sqrt(1.0f - sin2ThetaT);
//       float Fr = fresnel_dielectric(cosThetaI, etaI, etaT, cosThetaT);
//       refl.weight = Color(Fr);
//       samples.push_back(refl);

//       // Refraction ray
//       BxDFSample refr;
//       refr.wi = perturb(eta * -wo + (eta * cosThetaI - cosThetaT) * n, roughness, u);
//       refr.weight = Color(1.0f - Fr);
//       refr.is_transmission = true;
//       refr.is_valid = true;
//       samples.push_back(refr);
//     }
//   }
//   return samples;
// }

// Color Material::compute_brdf(const Vec3& wi, const Vec3& wo, const HitRecord& rec) const {
//   Vec3 n = rec.normal;
//   Vec3 h = glm::normalize(wi + wo);
//   float cos_theta_i = std::max(0.0f, glm::dot(n, wi));

//   // Diffuse Component
//   float diffuse_factor = brdf_cfg->normalized ? (1.0f / glm::pi<float>()) : 1.0f;
//   Color diffuse = this->diffuse_reflectance * diffuse_factor;

//   // Specular Component
//   Color specular(0.0f);
//   float p = brdf_cfg->exponent;

//   if (brdf_cfg->type == BRDFConfig::Type::ModifiedPhong ||
//       brdf_cfg->type == BRDFConfig::Type::OriginalPhong) {
//     Vec3 r = glm::reflect(-wi, n);
//     float cos_alpha = std::max(0.0f, glm::dot(r, wo));
//     float norm = brdf_cfg->normalized ? (p + 2.0f) / (2.0f * glm::pi<float>()) : 1.0f;
//     float denom = (brdf_cfg->type == BRDFConfig::Type::OriginalPhong) ? std::max(0.001f, cos_theta_i) : 1.0f;

//     specular = this->specular_reflectance * norm * glm::pow(cos_alpha, p) / denom;
//   }
//   else if (brdf_cfg->type == BRDFConfig::Type::ModifiedBlinnPhong ||
//            brdf_cfg->type == BRDFConfig::Type::OriginalBlinnPhong) {
//     float cos_alpha_h = std::max(0.0f, glm::dot(n, h));
//     float norm = brdf_cfg->normalized ? (p + 8.0f) / (8.0f * glm::pi<float>()) : 1.0f;
//     float denom = (brdf_cfg->type == BRDFConfig::Type::OriginalBlinnPhong) ? std::max(0.001f, cos_theta_i) : 1.0f;

//     specular = this->specular_reflectance * norm * glm::pow(cos_alpha_h, p) / denom;
//   }
//   else if (brdf_cfg->type == BRDFConfig::Type::TorranceSparrow) {
//     float cos_beta = std::max(0.0f, glm::dot(h, wi));
//     float D = ((p + 2.0f) / (2.0f * glm::pi<float>())) * glm::pow(std::max(0.0f, glm::dot(n, h)), p);
//     float G = smith_geometry(n, h, wo, wi);
//     float R0 = this->refraction_index;
//     float F = R0 + (1.0f - R0) * std::pow(1.0f - cos_beta, 5.0f);

//     specular = this->specular_reflectance * (D * F * G) / (4.0f * std::max(0.001f, glm::dot(n, wi)) * std::max(0.001f, glm::dot(n, wo)));
    
//     if (brdf_cfg->kd_fresnel) diffuse *= (1.0f - F);
//   }

//   return diffuse + specular;
// }

} // namespace hasmet

