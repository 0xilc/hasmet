#pragma once
#include <glm/glm.hpp>
#include <string>

#include "camera.h"
#include "core/ray.h"
#include "core/types.h"

namespace hasmet {
class ThinLensCamera : public Camera {
 public:
  ThinLensCamera(const Vec3& position,
                 const Vec3& look_at,
                 const Vec3& up,
                 float vertical_fov_degrees = 90.f,
                 int film_width = 100,
                 int film_height = 100,
                 std::string image_name = "thinlens_output",
                 const glm::mat4& transform = glm::mat4(1.0f),
                 int num_samples = 1,
                 float aperture_size = 0,
                 float focus_distance = 0);

  std::vector<Ray> generateRays(float px, float py) const override;

 private:
  Vec3 position_;
  Vec3 top_left_corner_;
  Vec3 horizontal_spacing_;
  Vec3 vertical_spacing_;
  Vec3 lens_center_;
  Vec3 u_;
  Vec3 v_;
  Vec3 w_;

  // Lens parameters
  float aperture_size_;
  float focus_distance_;

  // Sampling utils
  void generate_pixel_samples(int px, int py, std::vector<Vec3>& out) const;
  void generate_aperture_samples(std::vector<Vec3>& out) const;
};
} // namespace hasmet