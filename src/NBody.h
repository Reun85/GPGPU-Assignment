#pragma once

#define CL_TARGET_OPENCL_VERSION 200
#define CL_HPP_TARGET_OPENCL_VERSION 00
#define __NO_STD_VECTOR
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl_gl.h>
#include <GL/glew.h>

#include <CL/cl.hpp>
#include <cstddef>
#include <functional>
#include <glm/glm.hpp>
#include <mutex>
#include <optional>
#include <vector>

static const int PARTICLE_COUNT = 4000;

struct ParticleData {
  glm::vec3 velocity;
  // NOTE: this could be removed to decrease memory usage, however
  // then the m_writing_mutex has to be locked for the whole duration of the
  // Barnes-Hut algorithm which takes a while.
  // TODO: measure this.
  glm::vec3 force;

  float mass;
};

using ParticleSetDescription =
    std::pair<std::vector<glm::vec3>, std::vector<ParticleData>>;

ParticleSetDescription EvenLayout(const size_t size);
ParticleSetDescription UniformLayout(const size_t size);

class NBody {
 public:
  NBody();
  ~NBody();

  // Initialise OpenCL, attach OpenGL Buffers
  bool Init(GLuint VBOIndex);

  /// Generates the particles and moves them to the GPU.
  /// @param particle_count: the number passed to generating func
  /// @param extra_allocate_particle_count: the additional space allocated on
  /// the GPU to allow the user to add in particles after the simulation has
  /// started.
  void Start(
      const size_t particle_count = PARTICLE_COUNT,
      std::function<ParticleSetDescription(const size_t)> generating_func =
          UniformLayout,
      std::optional<const size_t> extra_allocate_particle_count = std::nullopt);

  // Clear Buffers
  // NOTE: MUST BE CALLED BEFORE CLEARING OpenGL Buffers!
  void Clean();

  // Update the simulationstate
  void Calculate();

  /// Attempts to write to the given OpenGL buffer
  void TryAndWriteData();

 private:
  //          ╭─────────────────────────────────────────────────────────╮
  //          │                  Simulation Constants                   │
  //          ╰─────────────────────────────────────────────────────────╯
  size_t particle_count;
  std::function<ParticleSetDescription(const size_t)> generating_function;
  float distanceThreshold = 0.2f;
  float eps = 1e-3f;
  double gravitational_constant = 6.67430e-11f;
  std::mutex m_sim_data_mutex;

  //          ╭─────────────────────────────────────────────────────────╮
  //          │                  Writing communication                  │
  //          ╰─────────────────────────────────────────────────────────╯
  std::mutex m_writing_mutex;
  std::mutex m_done_mutex;
  bool m_newdata = true;

  //          ╭─────────────────────────────────────────────────────────╮
  //          │                           CL                            │
  //          ╰─────────────────────────────────────────────────────────╯
  cl::Kernel boundingbox;
  cl::Kernel octree;
  cl::Kernel barneshut;
  cl::Kernel positionupdate;
};
