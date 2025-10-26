#pragma once
#include "core/spectrum.h"

enum class MaterialType{
	BlinnPhong,
	Mirror,
	Conductor,
	Dielectric
};


struct Material{
  MaterialType type = MaterialType::BlinnPhong;

  // Phong properties
  Spectrum ambient_reflectance{0.0f};
  Spectrum diffuse_reflectance{0.0f};
  Spectrum specular_reflectance{0.0f};
  float phong_exponent = 1.0f;

  //Recursive RT properties
  Spectrum mirror_reflectance{0.0f};
  Spectrum absorption_coefficient{0.0f};
  Spectrum absorption_index{0.0f};
  float refraction_index = 1.0f;
};