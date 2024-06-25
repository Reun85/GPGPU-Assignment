#include "Octree.h"

#include <cassert>
#include <cmath>
#include <iostream>

const std::array<vec3, 8> Node::OFFSETS = {vec3{-1.f, -1.f, -1.f},
                                           {1, -1, -1},
                                           {-1, 1, -1},
                                           {1, 1, -1},
                                           {-1, -1, 1},
                                           {1, -1, 1},
                                           {-1, 1, 1},
                                           {1, 1, 1}};

constexpr size_t mod8at(size_t inp, size_t j) {
  constexpr size_t mask = 7;
  return inp >> (3 * j) & mask;
}
constexpr Vec3<size_t> getIJK(const size_t id, const size_t depth) {
  size_t i = 0, j = 0, k = 0;
  size_t stride = 1;
  for (size_t d = 0; d < depth; d++) {
    size_t inner = mod8at(id, d);
    i += (inner & 1) * stride;
    j += ((inner >> 1) & 1) * stride;
    k += ((inner >> 2) & 1) * stride;
    stride *= 2;
  }
  return Vec3<size_t>(i, j, k);
}

bool isinside(vec3 inclusivemin, vec3 exclusivemax, vec3 inp) {
  return vec3::min(inclusivemin, inp) == inclusivemin &&
         vec3::max(exclusivemax, inp) == exclusivemax;
}

Octree::Octree(std::vector<ParticlePos> &particles,
               std::vector<ParticleData> &particle_data)
    : m_nodes(std::pow(8, START_DEPTH + 1) * 50 * 8) {
  m_root = &m_nodes[0];
  // on gpu code this has to be global with mutex
  size_t itr = 1;
  // Divide into START_DEPTH size
  start_ind = 0;
  size_t end_ind = 1;
  // START_DEPTH - 1, we will do the last block with Z indexing
  for (size_t i = 0; i < START_DEPTH - 1; i++) {
    for (size_t j = start_ind; j < end_ind; j++) {
      Node *current = &m_nodes[j];
      for (int k = 0; k < 8; k++) {
        current->m_children[k] = &m_nodes[itr];
        itr++;
      }
      current->m_data_type = NodeDataType::Parent;
    }
    start_ind = end_ind;
    end_ind = itr;
  }
  // Last
  size_t stride = static_cast<size_t>(std::pow(2, START_DEPTH));
  {
    size_t id = 0;
    size_t stridei = 1;
    size_t stridej = stride;
    size_t stridek = stride * stride;
    for (size_t j = start_ind; j < end_ind; j++) {
      Node *current = &m_nodes[j];
      for (int k = 0; k < 8; k++) {
        Vec3<size_t> other = getIJK(id, START_DEPTH);
        size_t index =
            other.x * stridei + other.y * stridej + other.z * stridek;
        current->m_children[k] = &m_nodes[end_ind + index];
        id++;
      }
      current->m_data_type = NodeDataType::Parent;
    }
    start_ind = end_ind;
    itr_start = id + end_ind;
  }

  Recalculate(particles, particle_data);
}

void Octree::Recalculate(std::vector<ParticlePos> &particles,
                         std::vector<ParticleData> &particles_data) {
  size_t stride = static_cast<size_t>(std::pow(2, START_DEPTH));
  size_t itr = itr_start;
  // Calculate the bounding box of that box of space
  m_bb.Recalculate(particles);
  m_center = m_bb.GetCenter();

  m_bb.AdjustToFitAll();
  vec3 full_min = m_bb.GetMin() - m_center;
  vec3 full_max = m_bb.GetMax() - m_center;
  m_size = full_max - full_min;
  for (Node n : m_nodes) {
    n.m_particle = nullptr;
    n.m_data_type = NodeDataType::Leaf;
    n.m_center_of_mass = vec3(0.f);
    n.m_mass = 0.f;
  }

  vec3 start_depth_size =
      m_size * static_cast<float>(std::pow(0.5f, START_DEPTH));

  m_nodes[0].m_region_size = m_size / 2.f;

  for (size_t i = 0; i < start_ind - 1; ++i) {
    for (int j = 0; j < 8; j++) {
      m_nodes[i].m_children[j]->m_region_size = m_nodes[i].m_region_size / 2.f;
    }
  }

  for (size_t id = 0; id < std::pow(8, START_DEPTH); id++) {
    // Since the space is split into 8^START_DEPTH, we can extract the i, j, k
    // coordinates as such:
    size_t tmp = id;

    size_t i = tmp & (stride - 1);
    tmp = tmp >> START_DEPTH;

    size_t j = tmp & (stride - 1);
    tmp = tmp >> START_DEPTH;
    size_t k = tmp & (stride - 1);
    tmp = tmp >> START_DEPTH;

    Node *currentstarter = &m_nodes[start_ind + id];

    vec3 start_depth_center =
        m_center + full_min +
        vec3(i * start_depth_size.x, j * start_depth_size.y,
             k * start_depth_size.z) +
        start_depth_size / 2.0f;

    for (size_t particleind = 0; particleind < particles.size();
         particleind++) {
      Node *current = currentstarter;
      bool done = false;
      vec3 current_block_center = start_depth_center;
      ParticlePos *const particle = &particles[particleind];
      ParticleData *const particle_data = &particles_data[particleind];
      vec3 particle_pos = particles[particleind];
      vec3 current_block_size = start_depth_size / 2.0f;
      // if not inside :)

      if (!isinside(current_block_center - current_block_size,
                    current_block_center + current_block_size, particle_pos)) {
        done = true;
      }
      while (!done) {
        //  Adjust data for node
        current->m_mass += particle_data->m_mass;
        current->m_center_of_mass += *particle * particle_data->m_mass;

        // Locate the particle in the box
        vec3 octant = vec3(particle_pos.x > current_block_center.x ? 1 : -1,
                           particle_pos.y > current_block_center.y ? 1 : -1,
                           particle_pos.z > current_block_center.z ? 1 : -1);
        // Get the index
        int index = (particle_pos.x > current_block_center.x ? 1 : 0) +
                    (particle_pos.y > current_block_center.y ? 2 : 0) +
                    (particle_pos.z > current_block_center.z ? 4 : 0);

        if (current->m_data_type == NodeDataType::Leaf) {
          // If it is a leaf
          if (current->m_particle == nullptr) {
            // If it is empty
            current->m_particle = particle;
            // This is default
            current->m_data_type = NodeDataType::Leaf;
            done = true;
          } else {
            current->m_data_type = NodeDataType::Parent;
            // If it is not empty
            ParticlePos *tmpparticle = current->m_particle;
            vec3 prev_particle_pos = *tmpparticle;
            // Calculate where the previous particle should go
            // Get the index
            int prev_index =
                (prev_particle_pos.x > current_block_center.x ? 1 : 0) +
                (prev_particle_pos.y > current_block_center.y ? 2 : 0) +
                (prev_particle_pos.z > current_block_center.z ? 4 : 0);
            itr += 8;
            if (itr >= m_nodes.size()) {
              std::cerr << "Not enough space";
              std::exit(1);
            }
            for (int m = 0; m < 8; m++) {
              current->m_children[m] = &m_nodes[itr - 8 + m];
              current->m_children[m]->m_particle = nullptr;
              current->m_children[m]->m_region_size =
                  current->m_region_size / 2.f;
            }
            Node *new_node_for_prev = current->m_children[prev_index];
            // This is default
            new_node_for_prev->m_data_type = NodeDataType::Leaf;
            new_node_for_prev->m_particle = tmpparticle;
            new_node_for_prev->m_mass = current->m_mass;
            new_node_for_prev->m_center_of_mass = current->m_center_of_mass;

            current_block_size /= 2.0f;
            current_block_center =
                current_block_center + (current_block_size)*octant;
            current = current->m_children[index];
          }
        } else {
          // If it is not a leaf
          current_block_size /= 2.0f;
          current_block_center =
              current_block_center + (current_block_size)*octant;
          current = current->m_children[index];
        }
      }
    }
  }
  std::cout << std::endl;

  assert(itr < m_nodes.size());
  // Calculate center of Mass
  // The ones before the start block are not properly calculated!

  for (size_t i = start_ind - 1; i != 0; --i) {
    Node &n = m_nodes[i];
    for (size_t j = 0; j < 8; ++j) {
      n.m_mass += n.m_children[j]->m_mass;
      n.m_center_of_mass += n.m_children[j]->m_center_of_mass;
    }
  }
  Node &n = m_nodes[0];
  for (size_t j = 0; j < 8; ++j) {
    n.m_mass += n.m_children[j]->m_mass;
    n.m_center_of_mass += n.m_children[j]->m_center_of_mass;
  }
  for (size_t i = 0; i < itr; ++i) {
    m_nodes[i].m_center_of_mass /= m_nodes[i].m_mass;
  }
  // std::cerr << "Allocated blocks: " << m_nodes.size() << "\nUsed: " << itr
  //           << std::endl;
}
