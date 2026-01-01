#pragma once

#include "core/types.h"

namespace hasmet {

class Image {
  public:
    Image(int width, int height, int channels, float* data);
    Image() = default;
    ~Image() = default;

    Color get_pixel(int x, int y) const;

    int get_width() const { return width_; }
    int get_height() const { return height_; }
    int get_channels() const { return channels_; }
  private:
    int width_ = 0;
    int height_ = 0;
    int channels_ = 0;

    std::vector<float> data_;
};
} // namespace hasmet