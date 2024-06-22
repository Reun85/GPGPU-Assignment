#pragma once

#include <GL/gl.h>

#include <cstddef>
#include <functional>
#include <glm/glm.hpp>
#include <mutex>
#include <vector>

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

struct SimulationData {
  const size_t particle_count;
  std::function<ParticleSetDescription(const size_t)> generating_function;
};

class NBody {
 public:
  NBody(
      const size_t size,
      std::function<ParticleSetDescription(const size_t)> fun = UniformLayout);
  ~NBody();

  // Initialise OpenCL, attach OpenGL Buffers
  void Init(GLuint VBOIndex);

  // Generates the particles and moves them to the GPU.
  void Start();

  // Clear Buffers
  // NOTE: MUST BE CALLED BEFORE CLEARING OpenGL Buffers!
  void Clean();

  // Update the simulationstate
  void Calculate();

  /// Attempts to write to the given OpenGL buffer
  void TryAndWriteData() {
    bool update = false;
    {
      std::lock_guard done_lock(m_done_mutex);
      if (m_newdata && m_writing_mutex.try_lock()) {
        m_newdata = false;
        update = true;
      }
      // Let go of the lock as early as possible
    }
    // We have new data and successfully locked m_writing_mutex
    if (update) {
      // TODO: Finish
      m_writing_mutex.unlock();
    }
  }

 private:
  SimulationData m_sim_data;
  std::mutex m_writing_mutex;
  std::mutex m_done_mutex;
  bool m_newdata = true;
  std::vector<ParticleData> m_particles_data;
  std::vector<glm::vec3> m_particles_pos;
};
