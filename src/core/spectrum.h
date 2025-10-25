#pragma once

#include <algorithm>
#include <iostream>

struct Spectrum {
  float r = 0, g = 0, b = 0;

  Spectrum() = default;
  Spectrum(float val) : r(val), g(val), b(val) {}
  Spectrum(float r, float g, float b) : r(r), g(g), b(b) {}

  Spectrum operator+(const Spectrum& other) const {
    return Spectrum(r + other.r, g + other.g, b + other.b);
  }
  Spectrum operator*(const Spectrum& other) const {
    return Spectrum(r * other.r, g * other.g, b * other.b);
  }
  Spectrum& operator*=(float s) {
    r *= s;
    g *= s;
    b *= s;
    return *this;
  }

  Spectrum operator/(float s) const { return Spectrum(r / s, g / s, b / s); }
  Spectrum& operator/=(float s) {
    r /= s;
    g /= s;
    b /= s;
    return *this;
  }

  void clamp(float min = 0.0f, float max = 1.0f) {
    r = std::clamp(r, min, max);
    g = std::clamp(g, min, max);
    b = std::clamp(b, min, max);
  }
};

inline Spectrum operator*(float s, const Spectrum& spec) { return spec * s; }

inline std::ostream& operator<<(std::ostream& os, const Spectrum& s) {
  return os << "Spectrum(" << s.r << ", " << s.g << ", " << s.b << ")";
}
