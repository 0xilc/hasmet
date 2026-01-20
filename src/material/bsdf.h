#pragma once

#include "bxdf.h"
#include "core/frame.h"
#include "core/hit_record.h"
#include "core/types.h"
#include <memory>
#include <vector>

namespace hasmet {
class BSDF {
public:
  BSDF(const HitRecord& rec) : frame(rec.normal) {}
  ~BSDF() {
    for (auto b : bxdfs) delete b;
  }
  
  void add(BxDF* b) { bxdfs.push_back(b); }

  Color f(const Vec3& woW, const Vec3& wiW) const {
    Vec3 wo = frame.to_local(woW);
    Vec3 wi = frame.to_local(wiW);
    Color result(0.0f);
    for (auto b : bxdfs) {
      result += b->f(wo, wi);
    }
    
    return result;
  }

  BxDFSample sample_f(const Vec3& woW, const Vec2& u) const {
    Vec3 wo = frame.to_local(woW);
    if (bxdfs.empty()) return {};
    
    int n = bxdfs.size();
    int index = std::min((int)(u.x * n), n - 1);

    Vec2 u_remapped(u.x * n - index, u.y);
    BxDFSample s = bxdfs[index]->sample_f(wo, u_remapped);
    
    if (s.pdf > 0) {
      s.wi = frame.to_world(s.wi);
      s.pdf /= n;
    }
    return s;
  }

  float pdf(const Vec3& woW, const Vec3& wiW) const{
    Vec3 wo = frame.to_local(woW);
    Vec3 wi = frame.to_local(wiW);
    float pdf_val = 0.0f;
    for (auto b : bxdfs) {
      pdf_val += b->pdf(wo, wi);
    }
    
    return bxdfs.empty() ? 0.0f : pdf_val / bxdfs.size();
  }

private:
  Frame frame;
  std::vector<BxDF*> bxdfs;
};
}