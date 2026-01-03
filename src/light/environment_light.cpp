#include "environment_light.h"
#include "core/hit_record.h"
#include "light/light.h"

namespace hasmet {

LightSample EnvironmentLight::sample_li(const HitRecord& rec, const Vec2& u) const {
  return LightSample{};
}
} // namespace hasmet