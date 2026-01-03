#include "point_light.h"
#include "core/types.h"
#include "glm/geometric.hpp"

namespace hasmet {

LightSample PointLight::sample_li(const Vec3& hit_point, const Vec2& u) const {
    Vec3 wi_full = this->position - hit_point;
    float dist = glm::length(wi_full);
    Vec3 wi = wi_full / dist;
    
    Color L = Color(this->intensity / (dist * dist));

    return { L, wi, 1.0f, dist };
}
} // namespace hasmet