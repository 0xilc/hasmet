#pragma once

#include "core/types.h"

namespace hasmet {
enum class MaterialType{
	BlinnPhong,
	Mirror,
	Conductor,
	Dielectric,
  TextureColor
};

struct Material{
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