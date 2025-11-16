#pragma once
#include <glm/glm.hpp>
#include <string>

#include "camera.h"
#include "core/ray.h"

class PinholeCamera : public Camera {
 public:
  PinholeCamera(const glm::vec3& position, const glm::vec3& look_at,
                const glm::vec3& up, float vertical_fov_degrees, int film_width,
                int film_height, std::string image_name,
                const glm::mat4& transform = glm::mat4(1.0f));

  Ray generateRay(float px, float py) const override;

  int film_width_;
  int film_height_;
  std::string image_name_;

 private:
  glm::vec3 position_;
  glm::vec3 top_left_corner_;
  glm::vec3 horizontal_spacing_;
  glm::vec3 vertical_spacing_;
};