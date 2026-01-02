#pragma once
#include "light.h"
#include "core/types.h"
#include <string>

namespace hasmet {

struct SphericalDirectionalLight : public Light {
  int image_id;
  std::string type;
  std::string sampler;

  SphericalDirectionalLight(int img_id, const std::string& t, const std::string& s)
      : image_id(img_id), type(t), sampler(s) {}

};

} // namespace hasmet