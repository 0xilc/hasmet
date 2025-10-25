#pragma once

#include "core/ray.h"

class Camera {
 public:
  virtual ~Camera() = default;
  
  //  The pixel coordinates (px, py) are in the image space [0, width-1] and [0,
  //  height-1]
  virtual Ray generateRay(float px, float py) const = 0;
};