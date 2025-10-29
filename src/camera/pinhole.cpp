#define _USE_MATH_DEFINES
#include "pinhole.h"
#include "core/ray.h"
#include <cmath>

PinholeCamera::PinholeCamera(const glm::vec3& position, const glm::vec3& look_at,
                             const glm::vec3& up, float vertical_fov_degrees,
                             int film_width, int film_height)
    : position_(position), film_width_(film_width), film_height_(film_height){
  glm::vec3 w = normalize(position - look_at);
  glm::vec3 u = normalize(cross(up, w));
  glm::vec3 v = -cross(w, u);

  float aspect_ratio =
      static_cast<float>(film_width) / static_cast<float>(film_height);
  float theta = vertical_fov_degrees * M_PI / 180.0f;
  float half_height = tan(theta / 2.0f);
  float half_width = aspect_ratio * half_height;

  lower_left_corner_ = position_ - half_width * u - half_height * v - w;
  horizontal_spacing_ = 2.0f * half_width * u / static_cast<float>(film_width);
  vertical_spacing_ = 2.0f * half_height * v / static_cast<float>(film_height);
}

Ray PinholeCamera::generateRay(float px, float py) const {
  glm::vec3 point_on_plane =
      lower_left_corner_ + px * horizontal_spacing_ + py * vertical_spacing_;
  glm::vec3 ray_direction = normalize(point_on_plane - position_);

  return Ray(position_, ray_direction);
}