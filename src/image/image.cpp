#include "image_manager.h"
#include "core/logging.h"

namespace hasmet {

Image::Image(int width, int height, int channels, unsigned char* data)
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
  float r = data_[index] / 255.0f;
  float g = data_[index + 1] / 255.0f;
  float b = data_[index + 2] / 255.0f;

  return Color(r, g, b);
}

} // namespace hasmet