#pragma once
#include <glm/glm.hpp>
#include <string>

#include "camera.h"
#include "core/ray.h"
#include "core/types.h"

namespace hasmet {
class PinholeCamera : public Camera {
 public:
  PinholeCamera(const Vec3& position, const Vec3& look_at, const Vec3& up,
                float vertical_fov_degrees, int film_width, int film_height,
                std::string image_name, int num_samples,
                const glm::mat4& transform = glm::mat4(1.0f));

  Ray generateRay(float px, float py, glm::vec2 u_pixel,
                  glm::vec2 u_lens) const override;

 private:
  Vec3 position_;
  Vec3 top_left_corner_;
  Vec3 horizontal_spacing_;
  Vec3 vertical_spacing_;

  // Sampling utils
  void generate_pixel_samples(int px, int py, std::vector<Vec3>& out) const;
};
}  // namespace hasmet