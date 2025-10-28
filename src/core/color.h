#pragma once

#include <algorithm>
#include <glm/vec3.hpp>  
#include <iostream>

struct Color {
  float r = 0, g = 0, b = 0;

  Color() = default;
  Color(float val) : r(val), g(val), b(val) {}
  Color(float r, float g, float b) : r(r), g(g), b(b) {}
  operator glm::vec3() const { return glm::vec3(r, g, b); }

  Color(const glm::vec3& v) : r(v.x), g(v.y), b(v.z) {}

  Color operator+(const Color& other) const {
    return Color(r + other.r, g + other.g, b + other.b);
  }
  Color& operator+=(const Color& other) {
    r += other.r;
    g += other.g;
    b += other.b;
    return *this;
  }

  Color operator*(const Color& other) const {
    return Color(r * other.r, g * other.g, b * other.b);
  }
  Color operator*(float s) const { return Color(r * s, g * s, b * s); }
  Color& operator*=(float s) {
    r *= s;
    g *= s;
    b *= s;
    return *this;
  }

  Color operator/(float s) const { return Color(r / s, g / s, b / s); }
  Color& operator/=(float s) {
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

inline Color operator*(float s, const Color& spec) { return spec * s; }

inline Color operator+(const glm::vec3& v, const Color& s) {
  return Color(v.x + s.r, v.y + s.g, v.z + s.b);
}

inline Color operator+(const Color& s, const glm::vec3& v) {
  return Color(s.r + v.x, s.g + v.y, s.b + v.z);
}

inline std::ostream& operator<<(std::ostream& os, const Color& s) {
  return os << "Spectrum(" << s.r << ", " << s.g << ", " << s.b << ")";
}
