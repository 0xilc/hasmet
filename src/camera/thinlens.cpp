#define _USE_MATH_DEFINES
#include "thinlens.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <random>

#include "core/ray.h"
#include "core/sampling.h"
#include "core/types.h"

namespace hasmet {
ThinLensCamera::ThinLensCamera(const Vec3& position,
                               const Vec3& look_at, const Vec3& up,
                               float vertical_fov_degrees, int film_width,
                               int film_height, std::string image_name,
                               const glm::mat4& transform, int num_samples,
                               float aperture_size, float focus_distance)
    : aperture_size_(aperture_size),
      focus_distance_(focus_distance) {
  num_samples_ = num_samples;
  film_width_ = film_width;
  film_height_ = film_height;
  image_name_ = image_name;
  const Vec3 initial_gaze = glm::normalize(look_at - position);
  const Vec3 final_position = transform * glm::vec4(position, 1.0f);
  const Vec3 final_up = transform * glm::vec4(up, 0.0f);
  const Vec3 transformed_gaze = transform * glm::vec4(initial_gaze, 0.0f);

  float zoom_factor = glm::length(transformed_gaze);
  if (zoom_factor < 1e-6f) {
    zoom_factor = 1.0f;
  }

  const Vec3 final_gaze_dir = transformed_gaze / zoom_factor;

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
  Vec3 view_plane_center = final_position - w_;

  position_ = final_position;
  top_left_corner_ = view_plane_center - half_width * u_ + half_height * v_;
  horizontal_spacing_ =
      (2.0f * half_width * u_) / static_cast<float>(film_width);
  vertical_spacing_ =
      (-2.0f * half_height * v_) / static_cast<float>(film_height);
  lens_center_ = view_plane_center;
}

std::vector<Ray> ThinLensCamera::generateRays(float px, float py) const {
  std::vector<Ray> rays;
  rays.reserve(num_samples_);

  std::vector<Vec3> pixel_samples;
  std::vector<Vec3> aperture_samples;
  pixel_samples.reserve(num_samples_);
  aperture_samples.reserve(num_samples_);

  generate_pixel_samples(px, py, pixel_samples);
  generate_aperture_samples(aperture_samples);

  static thread_local std::mt19937 generator(std::random_device{}());
  std::shuffle(aperture_samples.begin(), aperture_samples.end(), generator);

  Vec3 e = position_;

  for (int i = 0; i < num_samples_; i++) {
    Vec3 s = pixel_samples[i];
    Vec3 a = aperture_samples[i];
    Vec3 dir = s - e;
    float t_fp = focus_distance_ / glm::dot(dir, -w_);
    Vec3 p = e + t_fp * dir;
    Vec3 final_direction = glm::normalize(p - a);

    rays.emplace_back(a, final_direction);
  }
  
  return rays;
}

void ThinLensCamera::generate_pixel_samples(int px, int py,
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

void ThinLensCamera::generate_aperture_samples(
    std::vector<Vec3>& out) const {
  out.clear();

  const auto& jittered_samples =
      Sampling::generate_jittered_samples(num_samples_);

  for (const auto& [dx, dy] : jittered_samples) {
    Vec3 sample_point =
        position_ + (u_ * (dx - 0.5f) + v_ * (dy - 0.5f)) * aperture_size_;

    out.emplace_back(sample_point);
  }
}
} // namespace hasmet