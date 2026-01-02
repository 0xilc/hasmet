#pragma once

#include <string>
#include <vector>

#include "core/types.h"

namespace hasmet {
class Film {
 public:
  Film(int width, int height, const std::string& filename);
  Film operator=(Film other);
  
  void addSample(int x, int y, const Color& color);
  void write() const;
  int getWidth() const { return width_; }
  int getHeight() const { return height_; }
  
  int width_;
  int height_;
  std::string filename_;
  std::vector<Color> pixels_;
};
} // namespace hasmet