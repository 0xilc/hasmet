#define _USE_MATH_DEFINES
#include "pinhole.h"

#include <cmath>
#include <glm/gtc/constants.hpp>

#include "core/ray.h"
#include "core/sampling.h"

namespace hasmet {
PinholeCamera::PinholeCamera(const Vec3& position,
                             const Vec3& look_at, const Vec3& up,
                             float vertical_fov_degrees, int film_width,
                             int film_height, std::string image_name,
                             int num_samples, const glm::mat4& transform) {
  film_width_ = film_width;
  film_height_ = film_height;
  image_name_ = image_name;
  num_samples_ = num_samples;
  const Vec3 initial_gaze = glm::normalize(look_at - position);
  const Vec3 final_position = transform * glm::vec4(position, 1.0f);
  const Vec3 final_up = transform * glm::vec4(up, 0.0f);
  const Vec3 transformed_gaze = transform * glm::vec4(initial_gaze, 0.0f);

  float zoom_factor = glm::length(transformed_gaze);
  if (zoom_factor < 1e-6f) {
    zoom_factor = 1.0f;
  }

  const Vec3 final_gaze_dir =
      transformed_gaze / zoom_factor;  // glm::normalize(transformed_gaze)

  const Vec3 w = -final_gaze_dir;
  const Vec3 u = glm::normalize(glm::cross(final_up, w));
  const Vec3 v = glm::cross(w, u);

  const float aspect_ratio =
      static_cast<float>(film_width) / static_cast<float>(film_height);
  const float theta = glm::radians(vertical_fov_degrees);

  float half_height = tan(theta / 2.0f);
  half_height /= zoom_factor;

  float half_width = aspect_ratio * half_height;

  Vec3 view_plane_center = final_position - w;

  position_ = final_position;

  top_left_corner_ = view_plane_center - half_width * u + half_height * v;
  horizontal_spacing_ =
      (2.0f * half_width * u) / static_cast<float>(film_width);
  vertical_spacing_ =
      (-2.0f * half_height * v) / static_cast<float>(film_height);
}

std::vector<Ray> PinholeCamera::generateRays(float px, float py) const {
  std::vector<Ray> rays;
  rays.reserve(num_samples_);

  std::vector<Vec3> pixel_samples;
  pixel_samples.reserve(num_samples_);
  generate_pixel_samples(px, py, pixel_samples);

  Vec3 e = position_;

  for (int i = 0; i < num_samples_; i++) {
    Vec3 point_on_plane = pixel_samples[i];
    Vec3 ray_direction = glm::normalize(point_on_plane - position_);
    
    rays.emplace_back(position_, ray_direction);
  }

  return rays;
}

void PinholeCamera::generate_pixel_samples(int px, int py,
                                           std::vector<Vec3>& out) const {
  out.clear();

  const auto& jittered_samples =
      Sampling::generate_jittered_samples(num_samples_);

  for (const auto& [dx, dy] : jittered_samples) {
    Vec3 sample_point =
        top_left_corner_ + (static_cast<float>(px) + dx) * horizontal_spacing_ +
        (static_cast<float>(py) + dy) * vertical_spacing_;

    out.emplace_back(sample_point);
  }
}
} // namespace hasmet