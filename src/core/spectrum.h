#pragma once

#include <algorithm>
#include <glm/vec3.hpp>  
#include <iostream>

struct Spectrum {
  float r = 0, g = 0, b = 0;

  Spectrum() = default;
  Spectrum(float val) : r(val), g(val), b(val) {}
  Spectrum(float r, float g, float b) : r(r), g(g), b(b) {}
  operator glm::vec3() const { return glm::vec3(r, g, b); }

  Spectrum(const glm::vec3& v) : r(v.x), g(v.y), b(v.z) {}

  Spectrum operator+(const Spectrum& other) const {
    return Spectrum(r + other.r, g + other.g, b + other.b);
  }
  Spectrum& operator+=(const Spectrum& other) {
    r += other.r;
    g += other.g;
    b += other.b;
    return *this;
  }

  Spectrum operator*(const Spectrum& other) const {
    return Spectrum(r * other.r, g * other.g, b * other.b);
  }
  Spectrum operator*(float s) const { return Spectrum(r * s, g * s, b * s); }
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

inline Spectrum operator+(const glm::vec3& v, const Spectrum& s) {
  return Spectrum(v.x + s.r, v.y + s.g, v.z + s.b);
}

inline Spectrum operator+(const Spectrum& s, const glm::vec3& v) {
  return Spectrum(s.r + v.x, s.g + v.y, s.b + v.z);
}

inline std::ostream& operator<<(std::ostream& os, const Spectrum& s) {
  return os << "Spectrum(" << s.r << ", " << s.g << ", " << s.b << ")";
}
