typedef struct {
  float3 velocity;
  float3 force;
  float mass;
} ParticleData;

typedef struct {
  float3 center_of_mass;
  float3 region_size;
  int children[8];
  float mass;
  int isLeaf;
} Node;

#define isLeaf_LEAF 2
#define isLeaf_PARENT 1
#define isLeaf_EMPTY 0
__kernel void BarnesHut(__global const float3* particles_pos,
                        __global ParticleData* particles_data,
                        __global const Node* nodes, const int particle_count,
                        const float distanceThreshold, const float eps,
                        const float G, const int items_per_work_group) {
  const int global_id = get_global_id(0);

  const int start = global_id * items_per_work_group;

  const int end = min(start + items_per_work_group, particle_count);
  int stack[512];
  int stackSize = 0;
  for (int id = start; id < end; id++) {
    float3 particle_pos = particles_pos[id];
    ParticleData* particle_data = &particles_data[id];
    float3 force = (float3)(0, 0, 0);

    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        stack[stackSize++] = nodes[nodes[0].children[i]].children[j];
      }
    }

    while (stackSize > 0) {
      const Node* node = &nodes[stack[--stackSize]];

      if (node->mass == 0) continue;

      const float3 delta = node->center_of_mass - particle_pos;
      const float distance_squared =
          delta.x * delta.x + delta.y * delta.y + delta.z * delta.z + eps * eps;

      float biggestcomp = max(max(node->region_size.x, node->region_size.y),
                              node->region_size.z);
      float d_squared = biggestcomp * biggestcomp * 4.f / distance_squared;

      if (node->isLeaf == isLeaf_LEAF ||
          d_squared < distanceThreshold * distanceThreshold) {
        float F = (G * particle_data->mass * node->mass) / distance_squared;
        float3 add = delta * F / sqrt(distance_squared);
        force += add;
      } else {
        for (int j = 0; j < 8; ++j) {
          stack[stackSize++] = node->children[j];
        }
      }
    }
    particle_data->force = force;
  }
}

__kernel void AddForces(__global float3* particles_pos,
                        __global ParticleData* particles_data,
                        const int data_size, const int items_per_work_item,
                        const float dt) {
  int global_id = get_global_id(0);

  int start = global_id * items_per_work_item;
  int end = min(start + items_per_work_item, data_size);

  for (int id = start; id < end; id++) {
    float3* particle = &particles_pos[id];
    ParticleData* particle_data = &particles_data[id];

    particle_data->velocity += particle_data->force * dt / particle_data->mass;
    *particle += particle_data->velocity * dt;
  }
}

__kernel void BoundingBoxStage1(__global const float3* particles,
                                __global float3* min_values,
                                __global float3* max_values,
                                const int particle_count,
                                const int work_group_size) {
  int group_id = get_group_id(0);
  int local_id = get_local_id(0);
  int global_id = get_global_id(0);
  int num_groups = get_num_groups(0);

  __local float3 local_min[256];
  __local float3 local_max[256];

  float3 min_pos = (float3)(FLT_MAX, FLT_MAX, FLT_MAX);
  float3 max_pos = (float3)(-FLT_MAX, -FLT_MAX, -FLT_MAX);

  int items_per_work_item = (particle_count + num_groups - 1) / num_groups;
  int start = global_id * items_per_work_item;
  int end = min(start + items_per_work_item, particle_count);

  for (int i = start; i < end; i++) {
    float3 particle = particles[i];
    min_pos = fmin(min_pos, particle);
    max_pos = fmax(max_pos, particle);
  }

  local_min[local_id] = min_pos;
  local_max[local_id] = max_pos;

  barrier(CLK_LOCAL_MEM_FENCE);

  // Perform reduction to find min and max within the workgroup
  for (int offset = work_group_size / 2; offset > 0; offset >>= 1) {
    if (local_id < offset) {
      local_min[local_id] =
          fmin(local_min[local_id], local_min[local_id + offset]);
      local_max[local_id] =
          fmax(local_max[local_id], local_max[local_id + offset]);
    }
    barrier(CLK_LOCAL_MEM_FENCE);
  }

  if (local_id == 0) {
    min_values[group_id] = local_min[0];
    max_values[group_id] = local_max[0];
  }
}

__kernel void BoundingBoxStage2(__global const float3* min_values,
                                __global const float3* max_values,
                                __global float3* global_min,
                                __global float3* global_max,
                                const int work_group_size) {
  int local_id = get_local_id(0);

  __local float3 local_min[256];
  __local float3 local_max[256];

  local_min[local_id] = min_values[local_id];
  local_max[local_id] = max_values[local_id];

  barrier(CLK_LOCAL_MEM_FENCE);

  // Perform reduction to find global min and max
  for (int offset = work_group_size / 2; offset > 0; offset >>= 1) {
    if (local_id < offset) {
      local_min[local_id] =
          fmin(local_min[local_id], local_min[local_id + offset]);
      local_max[local_id] =
          fmax(local_max[local_id], local_max[local_id + offset]);
    }
    barrier(CLK_LOCAL_MEM_FENCE);
  }

  if (local_id == 0) {
    *global_min = local_min[0];
    *global_max = local_max[0];
  }
}

size_t mod8at(size_t inp, size_t j) {
  const size_t mask = 7;
  return inp >> (3 * j) & mask;
}

int3 getIJK(size_t id, size_t depth) {
  int3 res = (int3)(0, 0, 0);
  size_t stride = 1;
  for (size_t d = 0; d < depth; d++) {
    size_t inner = mod8at(id, d);
    res.x += (inner & 1) * stride;
    res.y += ((inner >> 1) & 1) * stride;
    res.z += ((inner >> 2) & 1) * stride;
    stride *= 2;
  }
  return res;
}

bool isinside(float3 inclusivemin, float3 exclusivemax, float3 inp) {
  return all(fmin(inclusivemin, inp) == inclusivemin) &&
         all(fmax(exclusivemax, inp) == exclusivemax);
}

// This only runs once
__kernel void CreateOctree(__global Node* nodes, const int start_depth) {
  // Initialize nodes
  size_t itr = 1;
  size_t start_ind = 0;
  size_t end_ind = 1;

  // START_DEPTH - 1, we will do the last block with Z indexing
  for (size_t i = 0; i < start_depth - 1; i++) {
    for (size_t j = start_ind; j < end_ind; j++) {
      Node* current = &nodes[j];
      for (int k = 0; k < 8; k++) {
        current->children[k] = itr;
        itr++;
      }
      current->isLeaf = isLeaf_PARENT;
    }
    start_ind = end_ind;
    end_ind = itr;
  }

  // Last
  size_t stride = 1 << start_depth;
  size_t id = 0;
  size_t stridei = 1;
  size_t stridej = stride;
  size_t stridek = stride * stride;

  for (size_t j = start_ind; j < end_ind; j++) {
    Node* current = &nodes[j];
    for (int k = 0; k < 8; k++) {
      int3 add = getIJK(id, start_depth);
      size_t index = add.x * stridei + add.y * stridej + add.z * stridek;
      current->children[k] = end_ind + index;
      id++;
    }
    current->isLeaf = isLeaf_EMPTY;
  }
}

size_t add8powers(const size_t exp) {
  size_t res = 0;
  for (size_t i = 0; i <= exp; i++) {
    res += 1 << (3 * i);
  }
  return res;
}

__kernel void InitOctree(__global Node* nodes, const int start_depth,
                         __global const float3* boundingbox_min,
                         __global const float3* boundingbox_max,
                         __global int* itr, const int allocatedNodes) {
  float3 center_of_universe = (*boundingbox_max + *boundingbox_min) / 2.0f;

  const float eps = 0.001f;
  // float3 full_min = *boundingbox_min - center_of_universe - eps;
  // float3 full_max = *boundingbox_max - center_of_universe + eps;
  // float3 size = full_max - full_min;
  float3 size = (*boundingbox_max) - (*boundingbox_min) + eps;

  nodes[0].region_size = size * 0.5f;

  *itr = add8powers(start_depth);
  size_t c = add8powers(start_depth - 1);
  for (size_t i = 0; i < c; ++i) {
    for (int j = 0; j < 8; j++) {
      nodes[nodes[i].children[j]].region_size = nodes[i].region_size * 0.5f;
    }
    nodes[i].isLeaf = isLeaf_PARENT;
    nodes[i].mass = 0.0f;
    nodes[i].center_of_mass = (float3)(0, 0, 0);
  }
  for (size_t i = c; i < *itr; ++i) {
    nodes[i].isLeaf = isLeaf_EMPTY;
    nodes[i].mass = 0.0f;
    nodes[i].center_of_mass = (float3)(0, 0, 0);
  }
}
// Kernel for initializing the octree
__kernel void BuildOctree(__global const float3* particles_pos,
                          __global const ParticleData* particles_data,
                          __global Node* nodes,
                          __global const float3* boundingbox_min,
                          __global const float3* boundingbox_max,
                          const int particle_count, const int start_depth,
                          __global int* itr, const int items_per_work_item
                          ) {
  int global_id = get_global_id(0);

  int start_box_index = global_id * items_per_work_item;
  int end_box_index =
      min(start_box_index + items_per_work_item, (1 << (3 * start_depth)));

  size_t stride = 1 << start_depth;

  float3 center_of_universe = (*boundingbox_max + *boundingbox_min) / 2.0f;
  const float eps = 0.001f;
  float3 full_min = *boundingbox_min - center_of_universe - eps * 0.5f;
  float3 full_max = *boundingbox_max - center_of_universe + eps * 0.5f;
  float3 size = (*boundingbox_max) - (*boundingbox_min) + eps;

  float3 start_depth_size = size * pow(0.5f, start_depth);

  int start_ind = add8powers(start_depth - 1);
  // This is where it truly begins
  for (size_t id = start_box_index; id < end_box_index; id++) {
    Node* currentstarter = &nodes[start_ind + id];

    // Since the space is split into 8^START_DEPTH, we can extract the i, j, k
    // coordinates as such:
    size_t tmp = id;
    const size_t i = tmp & (stride - 1);
    tmp = tmp >> start_depth;

    const size_t j = tmp & (stride - 1);
    tmp = tmp >> start_depth;
    const size_t k = tmp & (stride - 1);
    tmp = tmp >> start_depth;
    // We got its location in the world

    const float3 start_depth_center =
        center_of_universe + full_min +

        (float3)(i * start_depth_size.x, j * start_depth_size.y,
                 k * start_depth_size.z) +
        start_depth_size * 0.5f;

    for (size_t particleind = 0; particleind < particle_count; particleind++) {
      Node* current = currentstarter;
      bool done = false;
      float3 current_block_center = start_depth_center;
      const float3 particle_pos = particles_pos[particleind];
      const ParticleData* const particle_data = &particles_data[particleind];
      float3 current_block_size = start_depth_size / 2.0f;

      if (!isinside(current_block_center - current_block_size,
                    current_block_center + current_block_size, particle_pos)) {
        done = true;
      } 
      while (!done) {
        // Locate the particle in the box
        float3 octant =
            (float3)(particle_pos.x > current_block_center.x ? 1.0f : -1.0f,
                     particle_pos.y > current_block_center.y ? 1.0f : -1.0f,
                     particle_pos.z > current_block_center.z ? 1.0f : -1.0f);

        // Get the index
        int index = (particle_pos.x > current_block_center.x ? 1 : 0) +
                    (particle_pos.y > current_block_center.y ? 2 : 0) +
                    (particle_pos.z > current_block_center.z ? 4 : 0);

        if (current->isLeaf == isLeaf_LEAF) {
          current->isLeaf = isLeaf_PARENT;
          // Its only that one particle, its center of mass is that particle
          float3 prev_particle_pos = current->center_of_mass / current->mass;
          int prev_index =
              (prev_particle_pos.x > current_block_center.x ? 1 : 0) +
              (prev_particle_pos.y > current_block_center.y ? 2 : 0) +
              (prev_particle_pos.z > current_block_center.z ? 4 : 0);

          size_t previtr = atomic_add(itr, 8);

          for (int m = 0; m < 8; m++) {
            current->children[m] = previtr + m;
            Node* n = &nodes[previtr + m];
            // current->children[m]->children[0] = 0;
            n->region_size = current->region_size / 2.0f;
            n->isLeaf = isLeaf_EMPTY;
            n->mass = 0.0f;
            n->center_of_mass = (float3)(0, 0, 0);
          }

          Node* new_node_for_prev = &nodes[current->children[prev_index]];
          new_node_for_prev->isLeaf = isLeaf_LEAF;
          new_node_for_prev->center_of_mass = current->center_of_mass;
          new_node_for_prev->mass = current->mass;

          //  Adjust data for node
          current->mass += particle_data->mass;
          current->center_of_mass += particle_pos * particle_data->mass;

          // Move onto next node
          current_block_size = current_block_size * 0.5f;
          current_block_center =
              current_block_center + current_block_size * octant;
          current = &nodes[current->children[index]];
        } else if (current->isLeaf == isLeaf_EMPTY) {
          done = true;

          //  Adjust data for node
          current->mass = particle_data->mass;
          current->center_of_mass = particle_pos * particle_data->mass;

          current->isLeaf = isLeaf_LEAF;
        } else {
          //  Adjust data for node
          current->mass += particle_data->mass;
          current->center_of_mass += particle_pos * particle_data->mass;

          // Move onto next node
          current_block_size = current_block_size * 0.5f;
          current_block_center =
              current_block_center + current_block_size * octant;
          current = &nodes[current->children[index]];
        }
      }
    }
  }
}

__kernel void CalculateCenterOfMass(__global Node* nodes, const int start_depth,
                                    __global int* itr) {
  size_t global_id = get_global_id(0);

  int start_ind = add8powers(start_depth - 1);
  for (size_t i = start_ind - 1; i != 0; --i) {
    Node* n = &nodes[i];
    for (size_t j = 0; j < 8; ++j) {
      n->mass += nodes[n->children[j]].mass;
      n->center_of_mass += nodes[n->children[j]].center_of_mass;
    }
  }

  Node* n = &nodes[0];
  for (size_t j = 0; j < 8; ++j) {
    n->mass += nodes[n->children[j]].mass;
    n->center_of_mass += nodes[n->children[j]].center_of_mass;
  }

  for (size_t i = 0; i < *itr; ++i) {
    nodes[i].center_of_mass /= nodes[i].mass;
  }
}