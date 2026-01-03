#include "image_manager.h"
#include "core/logging.h"

namespace hasmet {

Image::Image(int width, int height, int channels, float* data)
    : width_(width), height_(height), channels_(channels) {
  if (data) {
    data_.assign(data, data + (width * height * channels));
  }
}

Color Image::get_pixel(int x, int y) const {
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x >= width_) x = width_ - 1;
  if (y >= height_) y = height_ - 1;

  int index = (y * width_ + x) * channels_;
  float r = data_[index];
  float g = data_[index + 1];
  float b = data_[index + 2];

  return Color(r, g, b);
}

Color Image::get_pixel_bilinear(float u, float v) const {
  float x = u * (width_ - 1);
  float y = v * (height_ - 1);

  int x0 = static_cast<int>(std::floor(x));
  int y0 = static_cast<int>(std::floor(y));
  int x1 = static_cast<int>(std::min(x0 + 1, width_ - 1));
  int y1 = static_cast<int>(std::min(y0 + 1, height_ - 1));

  float dx = x - x0;
  float dy = y - y0;

  Color c00 = get_pixel(x0, y0);
  Color c10 = get_pixel(x1, y0);
  Color c01 = get_pixel(x0, y1);
  Color c11 = get_pixel(x1, y1);
  
  return glm::mix(glm::mix(c00, c10, dx), glm::mix(c01, c11, dx), dy);
}
} // namespace hasmet