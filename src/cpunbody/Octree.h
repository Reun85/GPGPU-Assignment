#pragma once
#include <array>
#include <functional>
#include <memory>
#include <ostream>
#include <vector>

#include "BoundingBox.h"
#include "Particle.h"
#include "Vec3.hpp"

enum NodeDataType {
  /// It has children
  Parent,
  /// It has a single particle inside
  Leaf,
};

class Node {
 public:
  static const std::array<vec3, 8> OFFSETS;
  union {
    /*
     6 7
      2 3
     4 5
      0 1

    */
    Node *m_children[8];
    ParticlePos *m_particle = nullptr;
  };
  vec3 m_region_size;
  float m_mass = 0;
  vec3 m_center_of_mass;

  NodeDataType m_data_type = NodeDataType::Leaf;
};

// Default is char to have as little size as possible.
// C++ does not have empty, zero sized types.
/// Used for Depth First Travel of the data type
template <typename T = char>
class NodeWithExtraInfo {
 public:
  Node *inner;
  size_t depth;
  vec3 center;
  vec3 size;
  std::shared_ptr<NodeWithExtraInfo> parent = nullptr;
  /// Can be anything
  /// Must have a default constructor
  T user;
};

class Octree {
 public:
  // The default times it is cut before injecting elements
  static constexpr size_t START_DEPTH = 4;
  Node *m_root;
  BoundingBox m_bb;
  Octree(std::vector<ParticlePos> &particles,
         std::vector<ParticleData> &particle_data);
  void Recalculate(std::vector<ParticlePos> &particles,
                   std::vector<ParticleData> &particle_data);

 public:
  /// T default is bool to have as little size as possible.
  // C++ does not have empty, zero sized types.
  template <typename T = bool, bool onlyOnLeaves = false>
  void DepthFirstTravel(
      std::function<void(std::shared_ptr<NodeWithExtraInfo<T>>)> fun) const;

 public:
  std::vector<Node> m_nodes;
  vec3 m_size;
  vec3 m_center;

 private:
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
  vec3 allsize = m_size;
  stack.emplace_back(std::make_shared<NodeWithExtraInfo<T>>(
      NodeWithExtraInfo<T>{m_root, 0, m_center, allsize * 0.5f}));

  while (!stack.empty()) {
    MS current = stack.back();
    stack.pop_back();
    if (current->inner->m_data_type == NodeDataType::Parent) {
      vec3 currsize = current->size * 0.5f;
      for (int i = 0; i < 8; i++) {
        vec3 currcenter = current->center + (currsize * Node::OFFSETS[i]);

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
