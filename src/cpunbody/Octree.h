#pragma once
#include <array>
#include <ostream>
#include <vector>

#include "BoundingBox.h"
#include "Particle.h"

// Essentially just a tuple type.
template <typename T>
struct Vec3 {
  T x, y, z;
};
template <>
struct Vec3<float> {
  float x, y, z;
  constexpr Vec3(const glm::vec3 &vec) : x(vec.x), y(vec.y), z(vec.z) {}
};
template <>
struct Vec3<size_t> {
  size_t x, y, z;
  constexpr Vec3(const glm::vec3 &vec) : x(vec.x), y(vec.y), z(vec.z) {}
  constexpr Vec3(size_t x, size_t y, size_t z) : x(x), y(y), z(z) {}
  constexpr Vec3 operator+(const Vec3 &other) const {
    return Vec3(x + other.x, y + other.y, z + other.z);
  }
  constexpr Vec3 operator*(const size_t &other) const {
    return Vec3(x * other, y * other, z * other);
  }
  constexpr bool operator==(const Vec3 &other) const {
    return x == other.x && y == other.y && z == other.z;
  }
  constexpr glm::vec3 toglm() const { return glm::vec3(x, y, z); }
};

template <typename T>
std::ostream &operator<<(std::ostream &os, const Vec3<T> &vec) {
  os << "(" << vec.x << "," << vec.y << "," << vec.z << ")";
  return os;
}
class Node {
 public:
  static const std::array<glm::vec3, 8> OFFSETS;
  union {
    /*
     6 7
      2 3
     4 5
      0 1


    */
    Node *m_children[8];
    Particle *m_particle = nullptr;
  };
  bool m_is_leaf = true;
};

class Octree {
 public:
  // The default times it is cut before injecting elements
  static constexpr size_t START_DEPTH = 2;
  Node *m_root;
  BoundingBox m_bb;
  Octree(std::vector<Particle> &particles);

 private:
  // Worst case scenario is 8^START_DEPTH+2*particles.size()
  std::vector<Node> m_nodes;
  glm::vec3 m_size;
  glm::vec3 m_center;
  friend std::ostream &operator<<(std::ostream &os, const Octree &octree);
};
