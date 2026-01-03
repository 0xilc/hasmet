#pragma once

#include "core/hit_record.h"
#include "core/types.h"

namespace hasmet {
enum class MaterialType{
	BlinnPhong,
	Mirror,
	Conductor,
	Dielectric,
  Unlit
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

  MaterialType type = MaterialType::BlinnPhong;

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