#pragma once

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_TARGET_OPENCL_VERSION 300
#define CL_HPP_TARGET_OPENCL_VERSION 300
#include <GL/glew.h>

#include <CL/opencl.hpp>
#include <chrono>
#include <cstddef>
#include <exception>
#include <functional>
#include <glm/glm.hpp>
#include <mutex>
#include <vector>

static const int PARTICLE_COUNT = 1e+4;

class NBodyTimer {
 public:
  float Tick() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = now - prev;
    prev = now;
    return duration.count();
  }
  NBodyTimer() : prev(std::chrono::high_resolution_clock::now()) {}

 private:
  std::chrono::high_resolution_clock::time_point prev;
};

struct ParticleData {
  cl_float3 velocity;
  // NOTE: this could be removed to decrease memory usage, however
  // then the m_writing_mutex has to be locked for the whole duration of the
  // Barnes-Hut algorithm which takes a while.
  // TODO: measure this.
  cl_float3 force;

  bool operator==(const ParticleData& rhs) {
    return velocity.x == rhs.velocity.x && velocity.y == rhs.velocity.y &&
           velocity.z == rhs.velocity.z && force.x == rhs.force.x &&
           force.y == rhs.force.y && force.z == rhs.force.z;
  }
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

ParticleSetDescription GalaxiesClashing(const size_t size, const float);
ParticleSetDescription Galaxy(const size_t size, const float);
ParticleSetDescription UniformLayout(const size_t size, const float);

class NBody {
 public:
  NBody();
  ~NBody();

  // Initialise OpenCL, attach OpenGL Buffers
  bool Init(GLuint VBOIndex, const size_t particle_count,
            const size_t extra_allocate_particle_count = 0);

  /// Generates the particles and moves them to the GPU.
  /// @param particle_count: the number passed to generating func
  /// @param extra_allocate_particle_count: the additional space allocated on
  /// the GPU to allow the user to add in particles after the simulation has
  /// started.
  void Start(std::function<ParticleSetDescription(const size_t, const float)>
                 generating_func =GalaxiesClashing);

  // Clear Buffers
  // NOTE: MUST BE CALLED BEFORE CLEARING OpenGL Buffers!
  void Clean();

  // Update the simulationstate
  void Calculate(NBodyTimer&);

  /// Attempts to write to the given OpenGL buffer
  bool TryAndWriteData();

 private:
  void doTesting();
  // Testing
  std::vector<cl_float4> test_positions;
  std::vector<ParticleData> test_data;

  //          ╭─────────────────────────────────────────────────────────╮
  //          │                  Simulation Constants                   │
  //          ╰─────────────────────────────────────────────────────────╯
  size_t particle_count;
  size_t allocated_particle_count;
  std::function<ParticleSetDescription(const size_t)> generating_function;
  float distanceThreshold = 0.1f;
  float eps = 1e-3f;
  //float gravitational_constant = 6.67430e-11f;
  float gravitational_constant = 6.67430e-4f;
  std::mutex m_sim_data_mutex;

  size_t START_DEPTH = 4;

  float default_mass = 50000;

  //          ╭─────────────────────────────────────────────────────────╮
  //          │                  Writing communication                  │
  //          ╰─────────────────────────────────────────────────────────╯
  std::mutex m_writing_mutex;
  std::mutex m_done_mutex;
  bool m_newdata = true;

  //          ╭─────────────────────────────────────────────────────────╮
  //          │                           CL                            │
  //          ╰─────────────────────────────────────────────────────────╯

  const int allocatedNodes = ((1uLL << (3 * 5)) * 200uLL * 8uLL);

  const int boundingbox_work_group_size = 256;
  cl::Kernel boundingbox;
  cl::Kernel boundingboxstage2;

  cl::Kernel createOctree;
  cl::Kernel initOctree;
  const int octree_items_per_thread = 1;
  cl::Kernel buildOctree;

  const int center_of_mass_items_per_thread = 8;
  cl::Kernel centerofMass;

  const int divide_by_mass_threads = 512;
  cl::Kernel DivideByMass;

  const int barneshut_items_per_thread = 4;
  cl::Kernel barneshut;

  const int position_update_items_per_thread = 16;
  const float timestep = 0.1f;
  cl::Kernel positionupdate;

  cl::Context context;
  // Simulation command_queue
  cl::CommandQueue command_queue;
  //// Copy command_queue
  cl::CommandQueue copy_command_queue;
  cl::Program program;

  // CL buffers

  cl::BufferGL openGLparticlepos;
  std::vector<cl::Memory> GLbuffers;
  cl::Buffer particlepos;
  cl::Buffer particledata;
  cl::Buffer Nodes;
  // Intermediate CL buffers
  cl::Buffer itrBuffer;
  cl::Buffer minValuesBuffer;
  cl::Buffer maxValuesBuffer;
  cl::Buffer globalMinBuffer;
  cl::Buffer globalMaxBuffer;
};
