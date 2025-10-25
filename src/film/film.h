#pragma once

#include <string>
#include <vector>

#include "core/spectrum.h"

class Film {
 public:
  Film(int width, int height, const std::string& filename);

  void addSample(int x, int y, const Spectrum& color);

  void write() const;

  int getWidth() const { return width_; }
  int getHeight() const { return height_; }

 private:
  int width_;
  int height_;
  std::string filename_;
  std::vector<Spectrum> pixels_;
};