#include "film.h"

#include <string>
#include <vector>

#include "core/logging.h"
#include "io/image_io.h"
#include "core/types.h"

namespace hasmet {
Film::Film(int width, int height, const std::string& filename)
    : width_(width), height_(height), filename_(filename) {
  pixels_.resize(width * height, Color(0.0f));
}

Film Film::operator=(Film other) {
  if (this == &other) return *this;
  
  width_ = other.width_;
  height_ = other.height_;
  filename_ = other.filename_;
  pixels_ = other.pixels_;

  return *this;
}

void Film::addSample(int x, int y, const Color& color) {
  if (x < 0 || x >= width_ || y < 0 || y >= height_) {
    LOG_WARN("Attempted to write to out of bound pixel (" + std::to_string(x) +
             ", " + std::to_string(y) + ")");
    return;
  }

  int index = y * width_ + x;
  pixels_[index] = color;
}

void Film::write() const {
  LOG_INFO("Writing image to " + filename_ + "...");
  std::string ext = filename_.substr(filename_.find_last_of(".") + 1);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  bool success = false;
  if (ext == "exr") {
    success = write_exr(filename_, pixels_, width_, height_);
  }
  else {
    
    success = write_png(filename_, pixels_, width_, height_);
  }

  if (success) {
    LOG_INFO("Image: " << filename_ << " successfully written");
  } else {
    LOG_ERROR("Failed to write image to :" << filename_);
  }
}
} // namespace hasmet