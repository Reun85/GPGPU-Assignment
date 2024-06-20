#pragma once
#include <array>
#include <functional>
#include <memory>
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
enum NodeDataType {
  /// It has children
  Parent,
  /// It has a single particle inside
  Leaf,
  /// It has two or more combined particles. Treat it as a single particle
  CombinedLeaf
};

struct CombinedParticles {
  glm::vec3 m_position;
  /// doubles as mass
  int m_count;

 public:
  inline CombinedParticles() : m_position(0), m_count(0) {}
  inline CombinedParticles(const Particle &p)
      : m_position(p.m_position), m_count(1) {}
  inline void End() { m_position /= m_count; }
  inline void Add(const glm::vec3 &Pos) {
    m_position += Pos;
    m_count++;
  }
  inline void Add(const Particle &p) {
    m_position += p.m_position;
    m_count++;
  }
};
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
    CombinedParticles m_combined;
  };
  NodeDataType m_data_type = NodeDataType::Leaf;
};

/// Default is bool to have as little size as possible.
// C++ does not have empty, zero sized types.
template <typename T = bool>
class NodeWithExtraInfo {
 public:
  Node *inner;
  size_t depth;
  glm::vec3 center;
  glm::vec3 size;
  std::shared_ptr<NodeWithExtraInfo> parent = nullptr;
  /// Can be anything
  /// Must have a default constructor
  T user;
};

class Octree {
 public:
  // The default times it is cut before injecting elements
  static constexpr size_t START_DEPTH = 4;
  // After reaching this depth, it will just combine the particles.
  static constexpr size_t MAX_DEPTH = 20;
  static_assert(MAX_DEPTH > START_DEPTH);
  Node *m_root;
  BoundingBox m_bb;
  Octree(std::vector<Particle> &particles);
  void Recalculate(std::vector<Particle> &particles);

 public:
  /// T default is bool to have as little size as possible.
  // C++ does not have empty, zero sized types.
  template <typename T = bool, bool onlyOnLeaves = false>
  void DepthFirstTravel(
      std::function<void(std::shared_ptr<NodeWithExtraInfo<T>>)> fun) const;

 private:
  std::vector<Node> m_nodes;
  glm::vec3 m_size;
  glm::vec3 m_center;
  size_t start_ind;
  size_t itr_start;
  friend std::ostream &operator<<(std::ostream &os, const Octree &octree);
};

template <typename T, bool onlyOnLeaves>
void Octree::DepthFirstTravel(
    std::function<void(std::shared_ptr<NodeWithExtraInfo<T>>)> fun) const {
  /// Set sizes for Octree Nodes
  using MS = std::shared_ptr<NodeWithExtraInfo<T>>;
  std::vector<std::shared_ptr<NodeWithExtraInfo<T>>> stack;
  glm::vec3 allsize = m_size;
  stack.emplace_back(std::make_shared<NodeWithExtraInfo<T>>(
      NodeWithExtraInfo<T>{m_root, 0, m_center, 0.5f * allsize}));

  while (!stack.empty()) {
    MS current = stack.back();
    stack.pop_back();
    if (current->inner->m_data_type == NodeDataType::Parent) {
      glm::vec3 currsize = 0.5f * current->size;
      for (int i = 0; i < 8; i++) {
        glm::vec3 currcenter = current->center + (currsize * Node::OFFSETS[i]);

        stack.emplace_back(
            std::make_shared<NodeWithExtraInfo<T>>(NodeWithExtraInfo<T>{
                current->inner->m_children[i], current->depth + 1, currcenter,
                currsize, MS(current)}));
      }
    } else {
      if (onlyOnLeaves) {
        if (current->inner->m_data_type == NodeDataType::Leaf) {
          fun(current);
        }
      } else
        fun(current);
    }
  }
}
