#include "material.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include "core/hit_record.h"
#include "core/logging.h"
#include "core/ray.h"
#include "core/types.h"
#include "glm/exponential.hpp"
#include "glm/geometric.hpp"
#include "bxdf_library.h"
#include "material/bsdf.h"

namespace hasmet {

// TODO : fix specular reflection as it must have shininess coefficient too.
void BlinnPhongMaterial::setup_bsdf(HitRecord& rec, BSDF& bsdf) const {
  Color effective_kd = kd_;
  
  // Specular Component
  if (glm::length(ks_) > 1e-5f) {
    switch (brdf_config_.type) {
      case BRDFConfig::Type::OriginalBlinnPhong:
        bsdf.add(new BlinnPhongReflection(ks_, shininess_, false, false));
        break;
      case BRDFConfig::Type::OriginalPhong: 
        bsdf.add(new PhongReflection(ks_, shininess_, false, false));
        break;
      case BRDFConfig::Type::ModifiedBlinnPhong: 
        bsdf.add(new BlinnPhongReflection(ks_, shininess_, true, brdf_config_.normalized));
        break;
      case BRDFConfig::Type::ModifiedPhong:
        bsdf.add(new PhongReflection(ks_, shininess_, true, brdf_config_.normalized));
        break;
      case BRDFConfig::Type::TorranceSparrow: 
        bsdf.add(new MicrofacetReflection(ks_, shininess_, 1.5f));
        break;
    }
  }
  if (glm::length(kd_) > 1e-5f) {
    bsdf.add(new LambertianReflection(kd_, brdf_config_.normalized));
  }
}

void MirrorMaterial::setup_bsdf(HitRecord& rec, BSDF& bsdf) const {
  bsdf.add(new LambertianReflection(kd_));
  bsdf.add(new BlinnPhongReflection(ks_, p_, false, false));
  bsdf.add(new SpecularReflection(km_));
}

void ConductorMaterial::setup_bsdf(HitRecord& rec, BSDF& bsdf) const {
  bsdf.add(new BlinnPhongReflection(ks_, p_, false, false));
  bsdf.add(new ConductorReflection(eta_, k_, km_));
}

void DielectricMaterial::setup_bsdf(HitRecord& rec, BSDF& bsdf) const {
  bsdf.add(new DielectricReflection(ior_));
  bsdf.add(new SpecularTransmission(Color(1.0f), ior_));
}

void UnlitMaterial::setup_bsdf(HitRecord& rec, BSDF& bsdf) const {
  // TODO: DO texture lookup and evaluate the color and pass it.
  bsdf.add(new UnlitBxDF(color_));
}

} // namespace hasmet

