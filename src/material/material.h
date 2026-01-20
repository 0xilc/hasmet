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
  MirrorMaterial(Color reflection_coefficient, float roughness);

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

// struct BRDFConfig {
//   enum class Type {
//     OriginalPhong,
//     OriginalBlinnPhong,
//     ModifiedPhong,
//     ModifiedBlinnPhong,
//     TorranceSparrow
//   };
//   Type type;
//   bool normalized;
//   float exponent;
//   bool kd_fresnel;
// };

// struct BxDFSample {
//   Color weight{0.0f};
//   Vec3 wi{0.0f};
//   bool is_valid; // new ray generated?
//   bool is_transmission = false;
// };

// struct Material{
//   Color evaluate(const Vec3& wi, const Vec3& wo, const HitRecord& rec) const;
//   std::vector<BxDFSample> sample_f(const Vec3& wo, const HitRecord& rec, const glm::vec2& u);
//   Color compute_brdf(const Vec3& wi, const Vec3& wo, const HitRecord& rec) const;
//   MaterialType type = MaterialType::BlinnPhong;
//   std::optional<BRDFConfig> brdf_cfg;

//   // Phong properties
//   Color ambient_reflectance{0.0f};
//   Color diffuse_reflectance{0.0f};
//   Color specular_reflectance{0.0f};
//   float phong_exponent = 1.0f;

//   //Recursive RT properties
//   Color mirror_reflectance{0.0f};
//   Color absorption_coefficient{0.0f};
//   float absorption_index = 0.0f;
//   float refraction_index = 1.0f;
//   float roughness = 0.0f;
// };
} // namespace hasmet