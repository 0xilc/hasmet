#pragma once
#include <glm/glm.hpp>
#include <string>

#include "camera.h"
#include "core/ray.h"

class ThinLensCamera : public Camera {
 public:
  ThinLensCamera(const glm::vec3& position,
                 const glm::vec3& look_at,
                 const glm::vec3& up,
                 float vertical_fov_degrees = 90.f,
                 int film_width = 100,
                 int film_height = 100,
                 std::string image_name = "thinlens_output",
                 const glm::mat4& transform = glm::mat4(1.0f),
                 int num_samples = 1,
                 float aperture_size = 0,
                 float focus_distance = 0);

  Ray generateRay(float px, float py) const override;

  int film_width_;
  int film_height_;
  std::string image_name_;

 private:
  glm::vec3 position_;
  glm::vec3 top_left_corner_;
  glm::vec3 horizontal_spacing_;
  glm::vec3 vertical_spacing_;
  glm::vec3 lens_center_;
  glm::vec3 u_;
  glm::vec3 v_;
  glm::vec3 w_;

  // Lens parameters
  float aperture_size_;
  float focus_distance_;
  int num_samples_;
};