#pragma once

#include "ray.h"
#include "interval.h"

class AABB {
 public:
  Interval x, y, z;

  AABB() {};
  AABB(const glm::vec3& p1, const glm::vec3& p2) {
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
    glm::vec3 corners[8];
    corners[0] = glm::vec3(x.min, y.min, z.min);
    corners[1] = glm::vec3(x.min, y.min, z.max);
    corners[2] = glm::vec3(x.min, y.max, z.min);
    corners[3] = glm::vec3(x.min, y.max, z.max);
    corners[4] = glm::vec3(x.max, y.min, z.min);
    corners[5] = glm::vec3(x.max, y.min, z.max);
    corners[6] = glm::vec3(x.max, y.max, z.min);
    corners[7] = glm::vec3(x.max, y.max, z.max);
    glm::vec3 new_min(FLT_MAX);
    glm::vec3 new_max(-FLT_MAX);
    for (int i = 0; i < 8; i++) {
      glm::vec4 transformed = trns * glm::vec4(corners[i], 1.0f);
      glm::vec3 pt = glm::vec3(transformed) / transformed.w;
      new_min = glm::min(new_min, pt);
      new_max = glm::max(new_max, pt);
    }
    x = Interval(new_min.x, new_max.x);
    y = Interval(new_min.y, new_max.y);
    z = Interval(new_min.z, new_max.z);
  }

  bool intersect(Ray& ray) const {
    Interval tmp_interval = ray.interval_;
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
