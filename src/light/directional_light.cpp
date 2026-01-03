#include "directional_light.h"
#include "glm/geometric.hpp"

namespace hasmet {

LightSample DirectionalLight::sample_li(const Vec3& hit_point, const Vec2& u) const {
  Vec3 wi = glm::normalize(-this->direction);
  Color L = Color(this->radiance);

  return { L, wi, 1.0f, 1e20f };
}

} // namespace hasmet