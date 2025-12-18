#pragma once

#include "interval.h"
#include "ray.h"
#include "types.h"

namespace hasmet {
class AABB {
 public:
  Interval x, y, z;

  AABB() {};
  AABB(const Vec3& p1, const Vec3& p2) {
    x = Interval(p1.x, p2.x);
    y = Interval(p1.y, p2.y);
    z = Interval(p1.z, p2.z);
    thicken();
  }

  AABB(const AABB& box1, const AABB& box2) {
    x = Interval(box1.x, box2.x);
    y = Interval(box1.y, box2.y);
    z = Interval(box1.z, box2.z);
    thicken();
  }

  const void thicken() {
    x.thicken();
    y.thicken();
    z.thicken();
  }

  const Interval& axis(int i) const {
    if (i == 0) return x;
    if (i == 1)
      return y;
    else
      return z;
  }

  void apply_transformation(const glm::mat4& trns) {
    Vec3 corners[8];
    corners[0] = Vec3(x.min, y.min, z.min);
    corners[1] = Vec3(x.min, y.min, z.max);
    corners[2] = Vec3(x.min, y.max, z.min);
    corners[3] = Vec3(x.min, y.max, z.max);
    corners[4] = Vec3(x.max, y.min, z.min);
    corners[5] = Vec3(x.max, y.min, z.max);
    corners[6] = Vec3(x.max, y.max, z.min);
    corners[7] = Vec3(x.max, y.max, z.max);
    Vec3 new_min(FLT_MAX);
    Vec3 new_max(-FLT_MAX);
    for (int i = 0; i < 8; i++) {
      glm::vec4 transformed = trns * glm::vec4(corners[i], 1.0f);
      Vec3 pt = Vec3(transformed) / transformed.w;
      new_min = glm::min(new_min, pt);
      new_max = glm::max(new_max, pt);
    }
    x = Interval(new_min.x, new_max.x);
    y = Interval(new_min.y, new_max.y);
    z = Interval(new_min.z, new_max.z);
  }

  bool intersect(Ray& ray) const {
    Interval tmp_interval(ray.t_min, ray.t_max);
    for (int i = 0; i < 3; i++) {
      double t0 = (axis(i).min - ray.origin[i]) / ray.direction[i];
      double t1 = (axis(i).max - ray.origin[i]) / ray.direction[i];
      if (t0 > t1) std::swap(t0, t1);

      // check whether overlaps
      if (tmp_interval.min < t0) tmp_interval.min = t0;
      if (tmp_interval.max > t1) tmp_interval.max = t1;

      if (tmp_interval.max <= tmp_interval.min)
        return false;  // means no overlap between three axices.
    }
    return true;
  }

  Vec3 centroid() const {
    return Vec3((x.min + x.max) * 0.5f, (y.min + y.max) * 0.5f,
                     (z.min + z.max) * 0.5f);
  }

  void expand(const Vec3& p) {
    x.min = std::min(x.min, (double)p.x);
    x.max = std::max(x.max, (double)p.x);
    y.min = std::min(y.min, (double)p.y);
    y.max = std::max(y.max, (double)p.y);
    z.min = std::min(z.min, (double)p.z);
    z.max = std::max(z.max, (double)p.z);
  }

  void expand(const AABB& other) {
    x.min = std::min(x.min, other.x.min);
    x.max = std::max(x.max, other.x.max);
    y.min = std::min(y.min, other.y.min);
    y.max = std::max(y.max, other.y.max);
    z.min = std::min(z.min, other.z.min);
    z.max = std::max(z.max, other.z.max);
  }

  int longest_axis() const {
    double dx = x.max - x.min;
    double dy = y.max - y.min;
    double dz = z.max - z.min;

    if (dx > dy && dx > dz) return 0;
    if (dy > dz) return 1;
    return 2;
  }

  const Interval& operator[](int axis) const {
    switch (axis) {
      case 0:
        return x;
      case 1:
        return y;
      case 2:
        return z;
      default:
        throw std::out_of_range("Invalid axis index");
    }
  }
};

} // namespace hasmet