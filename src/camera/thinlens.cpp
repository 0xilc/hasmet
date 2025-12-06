#define _USE_MATH_DEFINES
#include "thinlens.h"

#include <cmath>
#include <glm/gtc/constants.hpp>

#include "core/ray.h"

ThinLensCamera::ThinLensCamera(const glm::vec3& position,
                               const glm::vec3& look_at, const glm::vec3& up,
                               float vertical_fov_degrees, int film_width,
                               int film_height, std::string image_name,
                               const glm::mat4& transform,
                               int num_samples, float aperture_size,
                               float focus_distance)
    : film_width_(film_width),
      film_height_(film_height),
      image_name_(image_name),
      aperture_size_(aperture_size),
      focus_distance_(focus_distance) {

  const glm::vec3 initial_gaze = glm::normalize(look_at - position);
  const glm::vec3 final_position = transform * glm::vec4(position, 1.0f);
  const glm::vec3 final_up = transform * glm::vec4(up, 0.0f);
  const glm::vec3 transformed_gaze = transform * glm::vec4(initial_gaze, 0.0f);

  float zoom_factor = glm::length(transformed_gaze);
  if (zoom_factor < 1e-6f) {
    zoom_factor = 1.0f;
  }

  const glm::vec3 final_gaze_dir = transformed_gaze / zoom_factor;

  // Calculate basis vectors
  w_ = -final_gaze_dir;
  u_ = glm::normalize(glm::cross(final_up, w_));
  v_ = glm::cross(w_, u_);

  const float aspect_ratio =
      static_cast<float>(film_width) / static_cast<float>(film_height);
  const float theta = glm::radians(vertical_fov_degrees);

  float half_height = tan(theta / 2.0f);
  half_height /= zoom_factor;

  float half_width = aspect_ratio * half_height;
  glm::vec3 view_plane_center = final_position - w_;

  position_ = final_position;
  top_left_corner_ = view_plane_center - half_width * u_ + half_height * v_;
  horizontal_spacing_ =
      (2.0f * half_width * u_) / static_cast<float>(film_width);
  vertical_spacing_ =
      (-2.0f * half_height * v_) / static_cast<float>(film_height);
  lens_center_ = view_plane_center;
}

Ray ThinLensCamera::generateRay(float px, float py) const {
  glm::vec3 s =
      top_left_corner_ + px * horizontal_spacing_ + py * vertical_spacing_;
  glm::vec3 e = position_;

  glm::vec3 lens_sample = e;
  if (aperture_size_ > 0) {
    float r1 = ((float)rand() / RAND_MAX) - 0.5f;
    float r2 = ((float)rand() / RAND_MAX) - 0.5f;
    lens_sample = e + (u_ * r1 + v_ * r2) * aperture_size_ / 2.f;
  }

  glm::vec3 dir = s - e;
  float t_fp = focus_distance_ / glm::dot(dir, -w_);
  glm::vec3 p = e + t_fp * dir;
  glm::vec3 final_direction = glm::normalize(p - lens_sample);

  return Ray(lens_sample, final_direction);
}