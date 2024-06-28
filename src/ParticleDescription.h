#pragma once

#include <CLPreComp.h>

struct ParticleData {
  cl_float3 velocity;
  // NOTE: this could be removed to decrease memory usage, however
  // then the m_writing_mutex has to be locked for the whole duration of the
  // Barnes-Hut algorithm which takes a while.
  // TODO: measure this.
  cl_float3 force;

  bool operator==(const ParticleData& rhs);
  bool operator!=(const ParticleData& rhs) { return !(*this == rhs); }
};

typedef struct {
  cl_float4 center_of_mass;
  cl_float3 region_size;
  cl_int isLeaf;  // 0 = isLeaf, 1 = Parent, 2 = Empty
  cl_int children[8];
  // Size is 68 bytes :c
  cl_int padding[3];
  // This way size is 80, which matches the alignment of the largest member
  // float3(float4) of 16 bytes.
} Node;

using ParticleSetDescription =
    std::pair<std::vector<cl_float4>, std::vector<ParticleData>>;
