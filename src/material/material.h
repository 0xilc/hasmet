#pragma once

#include "core/hit_record.h"
#include "core/types.h"
#include <memory>
#include "bsdf.h"

namespace hasmet {

// TODO: Refactor this, as it's only used by blinnphong materials.
struct BRDFConfig {
    enum class Type {
        OriginalBlinnPhong,
        OriginalPhong,
        ModifiedBlinnPhong,
        ModifiedPhong,
        TorranceSparrow
    };

    Type type = Type::OriginalBlinnPhong;
    bool normalized = false;
    float exponent = 1.0f;
    bool kd_fresnel = false;
};

class Material {
public:
  virtual ~Material() = default;

  virtual void setup_bsdf(HitRecord& rec, BSDF& bsdf) const = 0;
};

class MirrorMaterial : public Material {
public:
  MirrorMaterial(Color reflection_coefficient, float roughness)
    : R_(reflection_coefficient), roughness_(roughness) {};

  void setup_bsdf(HitRecord& rec, BSDF& bsdf) const override;
private:
  Color R_;
  float roughness_;
};

class BlinnPhongMaterial : public Material {
public:
  BlinnPhongMaterial(Color diffuse, Color specular, float phong_exponent, BRDFConfig brdf_cfg)
    : kd_(diffuse), ks_(specular), shininess_(phong_exponent) {}
  
  void setup_bsdf(HitRecord& rec, BSDF& bsdf) const override;
private:
  Color kd_;
  Color ks_;
  float shininess_;
  BRDFConfig brdf_config_;
};

class ConductorMaterial : public Material {
public:
  ConductorMaterial(Color index_refraction, Color absorption_coeff, float roughness) 
    : eta_(index_refraction), k_(absorption_coeff), roughness_(roughness){};
  
  void setup_bsdf(HitRecord& rec, BSDF& bsdf) const override;
private:
  Color eta_;          // Index of refraction
  Color k_;            // Absorption coefficient
  float roughness_;
};

class DielectricMaterial : public Material {
public:
  DielectricMaterial(float index_refraction, Color transmission_color = Color(1.0f))
    : ior_(index_refraction), transmittance_(transmission_color) {}

  void setup_bsdf(HitRecord& rec, BSDF& bsdf) const override;

private:
  float ior_;            // Index of Refraction
  Color transmittance_;  // Color tint of the glass
};

class UnlitMaterial : public Material {
public:
  UnlitMaterial(Color color, int texture_id = -1) 
    : color_(color), tex_id_(texture_id) {}
  
  void setup_bsdf(HitRecord& rec, BSDF& bsdf) const override;
private:
  Color color_;
  int tex_id_;
};
} // namespace hasmet