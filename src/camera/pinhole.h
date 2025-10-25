#pragma once

#include "camera.h"
#include "core/ray.h"
#include "core/vec.h"

class PinholeCamera : public Camera {
 public:
  PinholeCamera(const Vec3& position, const Vec3& look_at, const Vec3& up,
                float vertical_fov_degrees, int film_width, int film_height);

  Ray generateRay(float px, float py) const override;

 private:
  Vec3 position_;
  Vec3 lower_left_corner_;
  Vec3 horizontal_spacing_;
  Vec3 vertical_spacing_;
};