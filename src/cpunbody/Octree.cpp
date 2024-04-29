#include "Octree.h"

#include <cassert>
#include <cmath>
#include <iostream>

const std::vector<glm::vec3> Node::m_offsets = {
    {-1, -1, -1}, {1, -1, -1}, {-1, 1, -1}, {1, 1, -1},
    {-1, -1, 1},  {1, -1, 1},  {-1, 1, 1},  {1, 1, 1},
};

constexpr size_t mod8at(size_t inp, size_t j) {
  constexpr size_t mask = 7;
  return inp >> (3 * j) & mask;
}
constexpr Vec3<size_t> getIJK(const size_t id, const size_t depth) {
  size_t i = 0, j = 0, k = 0;
  int stride = 1;
  for (size_t d = 0; d < depth; d++) {
    int inner = mod8at(id, d);
    i += (inner & 1) * stride;
    j += ((inner >> 1) & 1) * stride;
    k += ((inner >> 2) & 1) * stride;
    stride *= 2;
  }
  return Vec3<size_t>(i, j, k);
}

bool isinside(glm::vec3 inclusivemin, glm::vec3 exclusivemax, glm::vec3 inp) {
  return glm::min(inclusivemin, inp) == inclusivemin &&
         glm::max(exclusivemax, inp) == exclusivemax;
}

std::ostream &operator<<(std::ostream &os, const Octree &octree) {
  /// Set sizes for Octree Nodes

  struct Stuff {
    Node *node;
    size_t depth;
    Vec3<size_t> ijk;
  };
  std::vector<Vec3<size_t>> strides = {
      {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0},
      {0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1},
  };
  os << "Octree: \n{\n";

  std::vector<Stuff> stack;
  stack.push_back({octree.m_root, 0, {0, 0, 0}});

  octree.m_root->center = octree.m_center;

  octree.m_root->size = octree.m_size;

  while (!stack.empty()) {
    Stuff current = stack.back();
    stack.pop_back();
    if (current.depth == Octree::START_DEPTH) {
      os << "Node: \n\t ijk:" << current.ijk << "\n\t"
         << Vec3<float>(current.node->center)
         << "\n\tprev: " << current.node->indices
         << "\n\tprev: " << Vec3<float>(current.node->prev_center) << std::endl;
    }
    if (!current.node->m_is_leaf) {
      for (int i = 0; i < 8; i++) {
        current.node->m_children[i]->center =
            current.node->center +
            0.25f * current.node->size * Node::m_offsets[i];
        current.node->m_children[i]->size = current.node->size * 0.5f;

        stack.push_back(
            {current.node->m_children[i], current.depth + 1,
             current.ijk + strides[i] * std::pow(2, Octree::START_DEPTH - 1 -
                                                        current.depth)});
      }
    } else {
      if (current.node->m_particle != nullptr) {
        os << "Particle: " << Vec3<float>(current.node->m_particle->m_position)
           << std::endl;
      }
    }
  }

  os << "}" << std::endl;
  os << "center" << Vec3<float>(octree.m_center) << std::endl;
  os << "size" << Vec3<float>(octree.m_size) << std::endl;

  return os;
}
void debug_TEST();

Octree::Octree(std::vector<Particle> &particles)
    : m_bb(particles),
      m_nodes(std::pow(8, START_DEPTH + 1) + 20 * particles.size()) {
#ifdef DEBUG
  debug_TEST();
#endif
  m_root = &m_nodes[0];
  // on gpu code this has to be global with mutex
  size_t itr = 1;
  // Divide into START_DEPTH size
  size_t start_ind = 0;
  size_t end_ind = 1;
  for (size_t i = 0; i < START_DEPTH; i++) {
    for (size_t j = start_ind; j < end_ind; j++) {
      Node *current = &m_nodes[j];
      for (int k = 0; k < 8; k++) {
        current->m_children[k] = &m_nodes[itr];
        itr++;
      }
      current->m_is_leaf = false;
    }
    start_ind = end_ind;
    end_ind = itr;
  }

  // Calculate the bounding box of that box of space
  m_center = m_bb.GetCenter();
  glm::vec3 full_min = m_bb.GetMin() - m_center - 0.01f;
  glm::vec3 full_max = m_bb.GetMax() - m_center + 0.01f;
  m_size = full_max - full_min;

  glm::vec3 start_depth_size =
      static_cast<float>(std::pow(0.5f, START_DEPTH)) * m_size;
  std::cout << Vec3<float>(start_depth_size) << std::endl;
  for (size_t id = 0; id < std::pow(8, START_DEPTH); id++) {
    // Since the space is split into 8^START_DEPTH, we can extract the i, j, k
    // coordinates as such:
    Vec3<size_t> other = getIJK(id, START_DEPTH);
    size_t i = other.x;
    size_t j = other.y;
    size_t k = other.z;
    Node *current = &m_nodes[start_ind + id];

    current->indices = other;

    glm::vec3 start_depth_center =
        m_center + full_min +
        glm::vec3(i * start_depth_size.x, j * start_depth_size.y,
                  k * start_depth_size.z) +
        start_depth_size / 2.0f;
    current->prev_center = start_depth_center;
    std::cout << "\nid: " << id
              << "\n\tcenter: " << Vec3<float>(start_depth_center)
              << "\n\t i,j,k: " << other << std::endl;

    for (int particleind = 0; particleind < particles.size(); particleind++) {
      bool done = false;
      glm::vec3 current_block_center = start_depth_center;
      Particle *const particle = &particles[particleind];
      const glm::vec3 particle_pos = particles[particleind].m_position;
      glm::vec3 current_block_size = start_depth_size / 2.0f;
      // if not inside :)

      if (!isinside(current_block_center - current_block_size,
                    current_block_center + current_block_size, particle_pos)) {
        done = true;
      }
      if (!done) {
        std::cout << "\t\tParticle: " << Vec3<float>(particle_pos)
                  << "  ParticleId: " << particleind << std::endl;
      }
      int iteratorcount = -1;
      while (!done) {
        iteratorcount++;
        std::cout << iteratorcount << std::endl;
        glm::vec3 octant =
            glm::vec3(particle_pos.x > current_block_center.x ? 1 : -1,
                      particle_pos.y > current_block_center.y ? 1 : -1,
                      particle_pos.z > current_block_center.z ? 1 : -1);
        // Get the index
        int index = (particle_pos.x > current_block_center.x ? 1 : 0) +
                    (particle_pos.y > current_block_center.y ? 2 : 0) +
                    (particle_pos.z > current_block_center.z ? 4 : 0);
        std::cout << "\tCurrent_center" << Vec3<float>(current_block_center)
                  << "\n\toctant: " << Vec3<float>(octant)
                  << " index: " << index << std::endl;

        if (current->m_is_leaf) {
          // If it is a leaf
          if (current->m_particle == nullptr) {
            // If it is empty
            current->m_particle = particle;
            // This is default
            current->m_is_leaf = true;
            done = true;
          } else {
            current->m_is_leaf = false;
            // If it is not empty
            Particle *tmpparticle = current->m_particle;
            glm::vec3 prev_particle_pos = tmpparticle->m_position;
            // Calculate where the previous particle should go
            // Get the index
            int prev_index =
                (prev_particle_pos.x > current_block_center.x ? 1 : 0) +
                (prev_particle_pos.y > current_block_center.y ? 2 : 0) +
                (prev_particle_pos.z > current_block_center.z ? 4 : 0);
            for (int m = 0; m < 8; m++) {
              current->m_children[m] = &m_nodes[itr];
              current->m_children[m]->m_particle = nullptr;
              itr++;
            }
            Node *new_node_for_prev = current->m_children[prev_index];
            // This is default
            new_node_for_prev->m_is_leaf = true;
            std::cout << "Conflict : " << Vec3<float>(tmpparticle->m_position)
                      << " prev index: " << prev_index << std::endl;
            new_node_for_prev->m_particle = tmpparticle;

            current_block_size /= 2.0f;
            current_block_center =
                current_block_center + (current_block_size / 2.0f) * octant;
            current = current->m_children[index];
          }
        } else {
          // If it is not a leaf
          current_block_size /= 2.0f;
          current_block_center =
              current_block_center + (current_block_size / 2.0f) * octant;
          current = current->m_children[index];
        }
      }
    }
  }

  assert(itr < m_nodes.size());
}

void debug_TEST() {
  std::vector<Vec3<size_t>> strides = {
      {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0},
      {0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1},
  };
  Vec3<size_t> start = {0, 0, 0};
  for (size_t id = 0; id < std::pow(8, Octree::START_DEPTH); id++) {
    Vec3 correct = start + strides[id % 8] + strides[(id / 8) % 8] * 2 +
                   strides[(id / 64) % 8] * 4;
    Vec3 got = getIJK(id, Octree::START_DEPTH);

    if (!(correct == got)) {
      std::cout << "\tBAD id: " << id << " correct: " << correct
                << " got: " << got << std::endl;
      assert(false);
    }
  }
  // for (int i = 0; i < 8; i++) {
  //   for (int j = 0; j < 8; j++) {
  //     std::cout << "i: " << i << " j: " << j << " " << getIJK(i * 8 + j, 2)
  //               << std::endl;
  //   }
  // }
}
