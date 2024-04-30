
#include <memory>

#include "Octree.h"

struct Depth {
  size_t depth;
  Depth(const size_t depth) : depth(depth){};
};

std::ostream &operator<<(std::ostream &os, const Depth &depth) {
  for (int i = 0; i < depth.depth; i++) os << "\t";
  return os;
}

struct Stuff {
  Node *node;
  size_t depth;
  glm::vec3 center;
  glm::vec3 size;
  bool printed = false;
  std::shared_ptr<Stuff> prev = nullptr;
};
std::ostream &operator<<(std::ostream &os, Stuff *current) {
  if (current->printed) return os;

  if (current->prev != nullptr) {
    os << current->prev;
  }
  os << Depth(current->depth) << "Node:"
     << "\n"
     << Depth(current->depth) << "center: " << Vec3<float>(current->center)
     << "\n"
     << Depth(current->depth) << "size: " << Vec3<float>(current->size) << "\n"
     << Depth(current->depth) << "children: " << std::endl;
  current->printed = true;

  return os;
}
std::ostream &operator<<(std::ostream &os, const Octree &octree) {
  /// Set sizes for Octree Nodes

  os << "Octree: \n{\n";

  using MS = std::shared_ptr<Stuff>;
  std::vector<std::shared_ptr<Stuff>> stack;
  glm::vec3 allsize = octree.m_size;
  stack.emplace_back(std::make_shared<Stuff>(
      Stuff{octree.m_root, 0, octree.m_center, 0.5f * allsize, true}));

  while (!stack.empty()) {
    MS current = stack.back();
    stack.pop_back();
    // if (current->depth == Octree::START_DEPTH) {
    //   os << current.get();
    // }
    if (current->depth == Octree::START_DEPTH) {
      os << current.get();
    }
    if (!current->node->m_is_leaf) {
      glm::vec3 currsize = 0.5f * current->size;
      for (int i = 0; i < 8; i++) {
        glm::vec3 currcenter = current->center + (currsize * Node::OFFSETS[i]);

        stack.emplace_back(std::make_shared<Stuff>(
            Stuff{current->node->m_children[i], current->depth + 1, currcenter,
                  currsize, false, MS(current)}));
      }
    } else {
      if (current->node->m_particle != nullptr) {
        os << current;
        os << Depth(current->depth + 1)
           << "Particle: " << Vec3<float>(current->node->m_particle->m_position)
           << std::endl;
      }
    }
  }

  os << "}" << std::endl;
  os << "center" << Vec3<float>(octree.m_center) << std::endl;
  os << "size" << Vec3<float>(octree.m_size) << std::endl;

  return os;
}
