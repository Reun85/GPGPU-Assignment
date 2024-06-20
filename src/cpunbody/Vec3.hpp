#pragma once
template <typename T>
struct Vec3 {
  T x, y, z;
  constexpr Vec3() : x(0), y(0), z(0) {}
  constexpr Vec3(const T& x) : x(x), y(y), z(z) {}
  constexpr Vec3(const T& x, const T& y, const T& z) : x(x), y(y), z(z) {}
  constexpr Vec3(const Vec3& other) : x(other.x), y(other.y), z(other.z) {}
  constexpr void operator=(const Vec3& other) {
    x = other.x;
    y = other.y;
    z = other.z;
  }
  constexpr Vec3(T&& x, T&& y, T&& z) : x(x), y(y), z(z) {}
  constexpr Vec3 operator+(const Vec3& other) const {
    return Vec3(x + other.x, y + other.y, z + other.z);
  }
  constexpr Vec3 operator+(const float& scalar) const {
    return Vec3(x + scalar, y + scalar, z + scalar);
  }
  constexpr Vec3 operator+=(const float& scalar) {
    x += scalar;
    y += scalar;
    z += scalar;
    return *this;
  }
  constexpr T smallest_component() const { return std::min(std::min(x, y), z); }
  constexpr T biggest_component() const { return std::max(std::max(x, y), z); }

  Vec3 operator-(const Vec3& other) const {
    return Vec3(x - other.x, y - other.y, z - other.z);
  }

  constexpr Vec3 operator-(const float& scalar) const {
    return Vec3(x - scalar, y - scalar, z - scalar);
  }
  constexpr Vec3 operator-=(const float& scalar) {
    x -= scalar;
    y -= scalar;
    z -= scalar;
    return *this;
  }

  constexpr Vec3 operator*(const Vec3& other) const {
    return Vec3(x * other.x, y * other.y, z * other.z);
  }
  Vec3 operator*(double scalar) const {
    return Vec3(x * scalar, y * scalar, z * scalar);
  }
  Vec3 operator*(float scalar) const {
    return Vec3(x * scalar, y * scalar, z * scalar);
  }
  Vec3 operator/(double scalar) const {
    return Vec3(x / scalar, y / scalar, z / scalar);
  }
  Vec3 operator/(float scalar) const {
    return Vec3(x / scalar, y / scalar, z / scalar);
  }
  constexpr Vec3 operator*=(const float& scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
  }
  constexpr Vec3 operator/=(const float& scalar) {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
  }
  constexpr static Vec3 min(const Vec3& a, const Vec3& b) {
    return Vec3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
  }
  constexpr static Vec3 max(const Vec3& a, const Vec3& b) {
    return Vec3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
  }

  Vec3& operator+=(const Vec3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }
  constexpr bool operator==(const Vec3& other) const {
    return x == other.x && y == other.y && z == other.z;
  }

  constexpr glm::vec3 toglm() const { return glm::vec3(x, y, z); }

  double magnitude() const { return std::sqrt(x * x + y * y + z * z); }

  Vec3 normalize() const {
    double mag = magnitude();
    return Vec3(x / mag, y / mag, z / mag);
  }
};

#include <iostream>
template <typename T>
std::ostream& operator<<(std::ostream& os, const Vec3<T>& vec) {
  os << "(" << vec.x << "," << vec.y << "," << vec.z << ")";
  return os;
}
template <typename T>
std::ostream& operator<<(std::ostream& os, const Vec3<T>* vec) {
  if (vec) os << "(" << vec->x << "," << vec->y << "," << vec->z << ")";
  return os;
}

using vec3 = Vec3<float>;