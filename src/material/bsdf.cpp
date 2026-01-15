#include "bsdf.h"
#include "core/hit_record.h"

namespace hasmet {

BSDF::BSDF(const HitRecord &rec) : frame(rec.normal) {}

void BSDF::add(const BxDF *bxdf) {
  if (count < MAX_BXDFS)
    bxdfs[count++] = bxdf;
}

Color BSDF::evaluate(const Vec3& wo_world, const Vec3& wi_world) const {
    Vec3 wo = frame.to_local(wo_world);
    Vec3 wi = frame.to_local(wi_world);
    if (wi.z * wo.z <= 0) return Color(0.0f);

    Color res(0.0f);
    for(int i = 0; i < count; ++i) res += bxdfs[i]->f(wo, wi);
    return res;
}

} // namespace hasmet