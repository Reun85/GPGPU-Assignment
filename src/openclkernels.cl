typedef struct {
  float3 velocity;
  float3 force;
  float mass;
} ParticleData;

typedef struct {
  float3 centerOfMass;
  float totalMass;
  float3 regionCenter;
  float regionSize;
  int children[8];
  int isLeaf;
} Node;
__kernel void BarnesHut(__global float3* particles_pos,
                        __global ParticleData* particles_data,
                        __global Node* nodes, int particle_count,
                        float distanceThreshold, float eps, float G) {
  int local_id = get_global_id(0);
  const int items_per_work_group = 16;

  int start = local_id * items_per_work_group;
  int end =min(start + items_per_work_group, particle_count);
  Node* stack[512];
  int stackSize = 0;
  for (int id = start; id < end; id++) {
    float3 particle_pos = *particles_pos[id];
    ParticleData* particle_data = particles_data + id;
    float3 force = float3(0, 0, 0);

    for (int i = 0; i < 8; i++)
      for (int j = 0; j < 8; j++)
        stack[stackSize++] = (*(*nodes[0]).children[i]).children[j];

    while (stackSize > 0) {
      Node* node = nodes+stack[--stackSize];

      if (node->m_mass == 0) continue;

      Vec3 delta = node->m_center_of_mass - particle_pos;
      float distance = sqrt(delta.x * delta.x + delta.y * delta.y +
                            delta.z * delta.z + eps * eps);

      float d = node->m_region_size.biggest_component() * 2 / distance;

      if (node->m_data_type == NodeDataType ::Leaf || d < distanceThreshold) {
        float F =
            (G * particle_data.m_mass * node->m_mass) / (distance * distance);
        vec3 add = delta * F / distance;
        force += add;
      } else {
        for (int j = 0; j < 8; ++j) {
          stack[stackSize++] = node->m_children[j];
        }
      }
    }
    particle_data.m_force = force;
  }
}
__kernel void AddForces(__global float3* particles_pos,
                        __global ParticleData* particles_data,
                        int particle_count, float dt) {
  int local_id = get_global_id(0);
  const int items_per_work_group = 16;

  int start = local_id * items_per_work_group;
  int end = std::min(start + items_per_work_group, particle_count);
  for (int id = start; id < end; id++) {
    float3 particle = *particles_pos[i];
    ParticleData* particle_data = particles_data + i;
    particle_data.m_velocity +=
        particle_data.m_force / particle_data.m_mass * dt;
    particle += particle_data.m_velocity * dt;
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

    // Local memory for storing min and max values within the workgroup
    __local float3 local_min[256];
    __local float3 local_max[256];

    // Initialize local min and max values
    float3 min = (float3)(FLT_MAX, FLT_MAX, FLT_MAX);
    float3 max = (float3)(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    // Calculate start and end indices for each work item
    int items_per_work_item = (particle_count + num_groups - 1) / num_groups;
    int start = global_id * items_per_work_item;
    int end = min(start + items_per_work_item, particle_count);

    // Find local min and max for each work item
    for (int i = start; i < end; i++) {
        float3 particle = particles[i];
        min = fmin(min, particle);
        max = fmax(max, particle);
    }

    // Store local min and max values to local memory
    local_min[local_id] = min;
    local_max[local_id] = max;

    // Synchronize local memory
    barrier(CLK_LOCAL_MEM_FENCE);

    // Perform reduction to find min and max within the workgroup
    for (int offset = work_group_size / 2; offset > 0; offset >>= 1) {
        if (local_id < offset) {
            local_min[local_id] = fmin(local_min[local_id], local_min[local_id + offset]);
            local_max[local_id] = fmax(local_max[local_id], local_max[local_id + offset]);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    // Write the results of the reduction to global memory
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

    // Local memory for storing min and max values within the final reduction
    __local float3 local_min[256];
    __local float3 local_max[256];

    // Load min and max values from global memory to local memory
    local_min[local_id] = min_values[local_id];
    local_max[local_id] = max_values[local_id];

    // Synchronize local memory
    barrier(CLK_LOCAL_MEM_FENCE);

    // Perform reduction to find global min and max
    for (int offset = work_group_size / 2; offset > 0; offset >>= 1) {
        if (local_id < offset) {
            local_min[local_id] = fmin(local_min[local_id], local_min[local_id + offset]);
            local_max[local_id] = fmax(local_max[local_id], local_max[local_id + offset]);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    // Write the results of the reduction to global memory
    if (local_id == 0) {
        *global_min = local_min[0];
        *global_max = local_max[0];
    }
}


#define FLT_MAX 3.402823466e+38F

// Function to calculate mod8at
size_t mod8at(size_t inp, size_t j) {
    const size_t mask = 7;
    return (inp >> (3 * j)) & mask;
}

// Function to calculate i, j, k indices
void getIJK(size_t id, size_t depth, size_t* i, size_t* j, size_t* k) {
    *i = 0;
    *j = 0;
    *k = 0;
    size_t stride = 1;
    for (size_t d = 0; d < depth; d++) {
        size_t inner = mod8at(id, d);
        *i += (inner & 1) * stride;
        *j += ((inner >> 1) & 1) * stride;
        *k += ((inner >> 2) & 1) * stride;
        stride *= 2;
    }
}

// Function to check if a point is inside a bounding box
bool isinside(float3 inclusivemin, float3 exclusivemax, float3 inp) {
    return all(fmin(inclusivemin, inp) == inclusivemin) &&
           all(fmax(exclusivemax, inp) == exclusivemax);
}

// Kernel for initializing the octree
__kernel void InitOctree(__global float3* particles_pos,
                         __global ParticleData* particles_data,
                         __global Node* nodes,
                         float3 boundingbox_min,
                         float3 boundingbox_max,
                         size_t particle_count,
                         size_t start_depth) {
    size_t global_id = get_global_id(0);
    size_t local_id = get_local_id(0);

    if (global_id >= particle_count) return;

    // Initialize the root node
    Node* root = &nodes[0];
    root->centerOfMass = (float3)(0, 0, 0);
    root->totalMass = 0.0f;
    root->regionCenter = (boundingbox_min + boundingbox_max) / 2.0f;
    root->regionSize = (boundingbox_max - boundingbox_min) / 2.0f;

    // Initialize particles
    for (size_t i = 0; i < particle_count; i++) {
        particles_data[i].force = (float3)(0, 0, 0);
    }

    // Initialize nodes
    size_t itr = 1;
    size_t start_ind = 0;
    size_t end_ind = 1;

    for (size_t i = 0; i < start_depth - 1; i++) {
        for (size_t j = start_ind; j < end_ind; j++) {
            Node* current = &nodes[j];
            for (int k = 0; k < 8; k++) {
                current->children[k] = &nodes[itr];
                itr++;
            }
            current->isLeaf = 0; // Mark as parent
        }
        start_ind = end_ind;
        end_ind = itr;
    }

    size_t stride = (size_t)pow(2, start_depth);
    size_t id = 0;
    size_t stridei = 1;
    size_t stridej = stride;
    size_t stridek = stride * stride;

    for (size_t j = start_ind; j < end_ind; j++) {
        Node* current = &nodes[j];
        for (int k = 0; k < 8; k++) {
            size_t i, j, k;
            getIJK(id, start_depth, &i, &j, &k);
            size_t index = i * stridei + j * stridej + k * stridek;
            current->children[k] = &nodes[end_ind + index];
            id++;
        }
        current->isLeaf = 0; // Mark as parent
    }

    start_ind = end_ind;
    size_t itr_start = id + end_ind;
    itr = itr_start;

    float eps = 0.001f;
    float3 full_min = boundingbox_min - root->regionCenter + eps;
    float3 full_max = boundingbox_max - root->regionCenter + eps;
    float3 size = full_max - full_min;

    float3 start_depth_size = size * pow(0.5f, start_depth);
    nodes[0].regionSize = size / 2.0f;

    for (size_t i = 0; i < start_ind - 1; ++i) {
        for (int j = 0; j < 8; j++) {
            nodes[i].children[j]->regionSize = nodes[i].regionSize / 2.0f;
        }
    }

    for (size_t id = 0; id < pow(8, start_depth); id++) {
        size_t tmp = id;
        size_t i = tmp & (stride - 1);
        tmp = tmp >> start_depth;

        size_t j = tmp & (stride - 1);
        tmp = tmp >> start_depth;
        size_t k = tmp & (stride - 1);
        tmp = tmp >> start_depth;

        Node* currentstarter = &nodes[start_ind + id];

        float3 start_depth_center =
            root->regionCenter + full_min +
            (float3)(i * start_depth_size.x, j * start_depth_size.y, k * start_depth_size.z) +
            start_depth_size / 2.0f;

        for (size_t particleind = 0; particleind < particle_count; particleind++) {
            Node* current = currentstarter;
            bool done = false;
            float3 current_block_center = start_depth_center;
            ParticlePos particle_pos = particles_pos[particleind];
            ParticleData* particle_data = &particles_data[particleind];
            float3 current_block_size = start_depth_size / 2.0f;

            if (!isinside(current_block_center - current_block_size,
                          current_block_center + current_block_size, particle_pos)) {
                done = true;
            }

            while (!done) {
                current->totalMass += particle_data->mass;
                current->centerOfMass += particle_pos * particle_data->mass;

                float3 octant = (float3)(particle_pos.x > current_block_center.x ? 1 : -1,
                                         particle_pos.y > current_block_center.y ? 1 : -1,
                                         particle_pos.z > current_block_center.z ? 1 : -1);

                int index = (particle_pos.x > current_block_center.x ? 1 : 0) +
                            (particle_pos.y > current_block_center.y ? 2 : 0) +
                            (particle_pos.z > current_block_center.z ? 4 : 0);

                if (current->isLeaf) {
                    if (current->children[0] == NULL) {
                        current->children[0] = particle_pos;
                        done = true;
                    } else {
                        current->isLeaf = 0;
                        ParticlePos tmpparticle = current->children[0];
                        float3 prev_particle_pos = tmpparticle;
                        int prev_index =
                            (prev_particle_pos.x > current_block_center.x ? 1 : 0) +
                            (prev_particle_pos.y > current_block_center.y ? 2 : 0) +
                            (prev_particle_pos.z > current_block_center.z ? 4 : 0);

                        itr += 8;
                        if (itr >= nodes.size()) {
                            printf("Not enough space\n");
                            return;
                        }

                        for (int m = 0; m < 8; m++) {
                            current->children[m] = &nodes[itr - 8 + m];
                            current->children[m]->children[0] = NULL;
                            current->children[m]->regionSize = current->regionSize / 2.0f;
                        }

                        Node* new_node_for_prev = current->children[prev_index];
                        new_node_for_prev->isLeaf = 1;
                        new_node_for_prev->children[0] = tmpparticle;
                        new_node_for_prev->totalMass = current->totalMass;
                        new_node_for_prev->centerOfMass = current->centerOfMass;

                        current_block_size /= 2.0f;
                        current_block_center += current_block_size * octant;
                        current = current->children[index];
                    }
                } else {
                    current_block_size /= 2.0f;
                    current_block_center += current_block_size * octant;
                    current = current->children[index];
                }
            }
        }
    }
}

__kernel void CalculateCenterOfMass(__global Node* nodes, size_t start_ind) {
    size_t global_id = get_global_id(0);

    for (size_t i = start_ind - 1; i != 0; --i) {
        Node n = nodes[i];
        for (size_t j = 0; j < 8; ++j) {
            n.totalMass += nodes[n.children[j]].totalMass;
            n.centerOfMass += nodes[n.children[j]].centerOfMass;
        }
    }

    Node n = nodes[0];
    for (size_t j = 0; j < 8; ++j) {
        n.totalMass += nodes[n.children[j]].totalMass;
        n.centerOfMass += nodes[n.children[j]].centerOfMass;
    }

    for (size_t i = 0; i < global_id; ++i) {
        nodes[i].centerOfMass /= nodes[i].totalMass;
    }
}
