#include "bxdf.h"
#include "core/logging.h"
#include "glm/geometric.hpp"
#include <glm/gtc/constants.hpp>

namespace hasmet {

OriginalBlinnPhongBRDF::OriginalBlinnPhongBRDF(const Color &kd, const Color &ks,
                                               float exponent)
    : BxDF(BxDFType(Diffuse | Glossy | Reflection)), kd_(kd), ks_(ks),
      exponent_(exponent) {}

Color OriginalBlinnPhongBRDF::f(const Vec3 &wo, const Vec3 &wi) const {
  Color diffuse = kd_ * glm::one_over_pi<float>();

  Vec3 h = glm::normalize(wo + wi);
  float cos_nh = std::max(0.0f, h.z);
  float cos_ni = std::max(0.001f, wi.z);

  Color specular = ks_ * glm::pow(cos_nh, exponent_) / cos_ni;

  return diffuse + specular;
}

float OriginalBlinnPhongBRDF::pdf(const Vec3 &wo, const Vec3 &wi) const {
  // Cosine weighted pdf
  return wi.z > 0 ? wi.z * glm::one_over_pi<float>() : 0;
}

Color OriginalBlinnPhongBRDF::sample_f(const Vec3 &wo, Vec3 *wi,
                                       const glm::vec2 &u,
                                       float *pdf_val) const {
  LOG_ERROR("Original bf sample_f not implemented yet!");
  return Color{0.0f};
}
} // namespace hasmet