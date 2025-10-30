#pragma once

#include <glm/glm.hpp>

#include "camera.h"
#include "core/ray.h"

class PinholeCamera : public Camera {
 public:
  PinholeCamera(const glm::vec3& position, const glm::vec3& look_at,
                const glm::vec3& up, float vertical_fov_degrees, int film_width,
                int film_height, std::string image_name);

  Ray generateRay(float px, float py) const override;

  // TODO: seperate this to film class
  int film_width_;
  int film_height_;
  std::string image_name_;

 private:
  glm::vec3 position_;
  glm::vec3 lower_left_corner_;
  glm::vec3 horizontal_spacing_;
  glm::vec3 vertical_spacing_;
};