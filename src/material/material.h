#pragma once

#include "core/hit_record.h"
#include "core/types.h"
#include <memory>

namespace hasmet {
enum class MaterialType{
	BlinnPhong,
	Mirror,
	Conductor,
	Dielectric,
  Unlit
};

struct BRDFConfig {
  enum class Type {
    OriginalPhong,
    OriginalBlinnPhong,
    ModifiedPhong,
    ModifiedBlinnPhong,
    TorranceSparrow
  };
  Type type;
  bool normalized;
  float exponent;
  bool kd_fresnel;
};

struct BxDFSample {
  Color weight{0.0f};
  Vec3 wi{0.0f};
  bool is_valid; // new ray generated?
  bool is_transmission = false;
};

struct Material{
  Color evaluate(const Vec3& wi, const Vec3& wo, const HitRecord& rec) const;
  std::vector<BxDFSample> sample_f(const Vec3& wo, const HitRecord& rec, const glm::vec2& u);
  Color compute_brdf(const Vec3& wi, const Vec3& wo, const HitRecord& rec) const;
  MaterialType type = MaterialType::BlinnPhong;
  std::optional<BRDFConfig> brdf_cfg;

  // Phong properties
  Color ambient_reflectance{0.0f};
  Color diffuse_reflectance{0.0f};
  Color specular_reflectance{0.0f};
  float phong_exponent = 1.0f;

  //Recursive RT properties
  Color mirror_reflectance{0.0f};
  Color absorption_coefficient{0.0f};
  float absorption_index = 0.0f;
  float refraction_index = 1.0f;
  float roughness = 0.0f;
};
} // namespace hasmet