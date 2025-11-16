#define _USE_MATH_DEFINES
#include "pinhole.h"

#include <cmath>
#include <glm/gtc/constants.hpp> 

#include "core/ray.h"

PinholeCamera::PinholeCamera(const glm::vec3& position,
                             const glm::vec3& look_at, const glm::vec3& up,
                             float vertical_fov_degrees, int film_width,
                             int film_height, std::string image_name,
                             const glm::mat4& transform)
    : film_width_(film_width),
      film_height_(film_height),
      image_name_(image_name) {

  const glm::vec3 initial_gaze = glm::normalize(look_at - position);
  const glm::vec3 final_position = transform * glm::vec4(position, 1.0f);
  const glm::vec3 final_up = transform * glm::vec4(up, 0.0f);
  const glm::vec3 transformed_gaze = transform * glm::vec4(initial_gaze, 0.0f);

  float zoom_factor = glm::length(transformed_gaze);
  if (zoom_factor < 1e-6f) { 
    zoom_factor = 1.0f;
  }

 
  const glm::vec3 final_gaze_dir =
      transformed_gaze / zoom_factor;  // glm::normalize(transformed_gaze)

  const glm::vec3 w = -final_gaze_dir;  
  const glm::vec3 u = glm::normalize(
      glm::cross(final_up, w));         
  const glm::vec3 v = glm::cross(w, u); 


  const float aspect_ratio =
      static_cast<float>(film_width) / static_cast<float>(film_height);
  const float theta = glm::radians(vertical_fov_degrees);

  float half_height = tan(theta / 2.0f);
  half_height /= zoom_factor;  

  float half_width = aspect_ratio * half_height;

  glm::vec3 view_plane_center = final_position - w;

  position_ = final_position;

  top_left_corner_ = view_plane_center - half_width * u + half_height * v;
  horizontal_spacing_ =
      (2.0f * half_width * u) / static_cast<float>(film_width);
  vertical_spacing_ =
      (-2.0f * half_height * v) / static_cast<float>(film_height);
}

Ray PinholeCamera::generateRay(float px, float py) const {
  glm::vec3 point_on_plane =
      top_left_corner_ + px * horizontal_spacing_ + py * vertical_spacing_;
  glm::vec3 ray_direction = normalize(point_on_plane - position_);

  return Ray(position_, ray_direction);
}