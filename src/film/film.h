#pragma once

#include <string>
#include <vector>

#include "core/logging.h"
#include "core/spectrum.h"
#include "io/image_io.h"

class Film {
 public:
  Film(int width, int height, const std::string& filename)
      : width_(width), height_(height), filename_(filename) {
    pixels_.resize(width * height, Spectrum(0.0f));
  }

  void addSample(int x, int y, const Spectrum& color) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
      LOG_WARN("Attempted to write to out of bound pixel (" +
               std::to_string(x) + ", " + std::to_string(y) + ")");
      return;
    }

    int index = y * width_ + x;
    pixels_[index] = color;
  }

  void write() const {
    LOG_INFO("Writing image to " + filename_ + "...");
    bool success = write_png(filename_, pixels_, width_, height_);

    if (success) {
      LOG_INFO("Image " + filename_ + " successfully written.");
    } else {
      LOG_INFO("Failed to write image to " + filename_);
    }
  }

 private:
  int width_;
  int height_;
  std::string filename_;
  std::vector<Spectrum> pixels_;
};