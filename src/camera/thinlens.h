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
                 float vertical_fov_degrees,
                 int film_width, 
                 int film_height, 
                 std::string image_name,
                 const glm::mat4& transform = glm::mat4(1.0f));

  Ray generateRay(float px, float py) const override;

  int film_width_;
  int film_height_;
  std::string image_name_;

 private:
  // Camera basis
  glm::vec3 position_;
  glm::vec3 forward_;
  glm::vec3 right_;
  glm::vec3 up_;

  // Lens parameters
  float focus_distance_;
  float aperture_size_;

  // Projection
  float near_plane_left_;
  float near_plane_right_;
  float near_plane_bottom_;
  float near_plane_top_;
  float near_distance_;

  // Sampling
  int num_samples_;
};