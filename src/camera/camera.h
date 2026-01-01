#pragma once

#include "core/ray.h"

namespace hasmet {
struct Tonemap{
    enum class Type {
        NONE,
        PHOTOGRAPHIC,
        FILMIC,
        ACES
    };
    Type type = Type::NONE;
    float options[2] = {1.0f, 1.0f};
    float saturation = 1.0f;
    float gamma = 2.2f;
    std::string extension = "_phot.png";
};

class Camera {
 public:
  virtual ~Camera() = default;

  //  The pixel coordinates (px, py) are in the image space [0, width-1] and [0,
  //  height-1]
  // u_pixel: Antialiasing (0-1), u_lens: DOF (0-1)
  virtual Ray generateRay(float px, float py, glm::vec2 u_pixel,
                           glm::vec2 u_lens) const = 0;

  int num_samples_;
  int film_width_;
  int film_height_;
  std::string image_name_;
  std::vector<Tonemap> tonemaps_;
};
}  // namespace hasmet