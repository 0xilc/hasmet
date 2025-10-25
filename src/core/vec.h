#pragma once
#include <cmath> 
#include <iostream>

struct Vec3 {
   float x = 0, y = 0, z = 0;

   Vec3() = default;
   Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

   // Vector-vector operations
   Vec3 operator+(const Vec3& other) const {
       return Vec3(x + other.x, y + other.y, z + other.z);
   };
   Vec3 operator-(const Vec3& other) const {
       return Vec3(x - other.x, y - other.y, z - other.z);
   };
   Vec3 operator*(const Vec3& other) const {
       return Vec3(x + other.x, y + other.y, z + other.z);
   };
   Vec3 operator-() const { return Vec3(-x, -y, -z); };

   // Vector-scalar operations
   Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
   Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }

   Vec3& operator+=(const Vec3& other) { x += other.x; y += other.y; z += other.z; return *this; }
   Vec3& operator-=(const Vec3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
   Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
   Vec3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }

   float length_squared() const { return x * x + y * y + z * z;}
   float length() const { return std::sqrt(length_squared()); }
};

inline Vec3 operator*(float s, const Vec3& v) { return v * s; }
inline float dot(const Vec3& u, const Vec3& v) { return u.x * v.x + u.y * v.y + u.z * v.z; }
inline Vec3 cross(const Vec3& u, const Vec3& v) {
    return Vec3(u.y * v.z - u.z * v.y,
        u.z * v.x - u.x * v.z,
        u.x * v.y - u.y * v.x);
}
inline Vec3 normalize(const Vec3& v) { return v / v.length(); }

inline std::ostream& operator<<(std::ostream& os, const Vec3& v) {
    return os << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
}


struct Vec2 {
    float x = 0, y = 0;

    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}
};