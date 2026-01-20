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

// TODO : fix specular reflection as it must have shininess coefficient too.
void BlinnPhongMaterial::setup_bsdf(HitRecord& rec, BSDF& bsdf) const {
  Color effective_kd = kd_;

  // Specular Component
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
  
  bsdf.add(new LambertianReflection(effective_kd));
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

} // namespace hasmet

