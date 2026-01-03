#pragma once
#include "core/hit_record.h"
#include "light.h"
#include <string>

namespace hasmet {

struct EnvironmentLight : public Light {
  int image_id;
  std::string type;
  std::string sampler;

  EnvironmentLight(int img_id, const std::string& t, const std::string& s)
      : image_id(img_id), type(t), sampler(s) {}

  LightSample sample_li(const HitRecord& rec, const Vec2& u) const override;
  Color sample_le(const Ray& ray) const;
};

} // namespace hasmet