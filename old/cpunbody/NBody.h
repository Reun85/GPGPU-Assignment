#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>

// std
#include <vector>

// Utils
#include <mutex>
#include <optional>
#include <ostream>

#include "Octree.h"
#include "Particle.h"
#include "SUpdateInfo.h"

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

class MyMutex {
 public:
  /// <summary>
  /// m Must be a locked mutex
  /// </summary>
  /// <param name="m"></param>
  /// <param name="data"></param>
  MyMutex(std::mutex& m, const std::vector<ParticlePos>& data)
      : m_mut(m), data(data) {}
  ~MyMutex() { m_mut.unlock(); }
  MyMutex(const MyMutex&) = delete;

 public:
  const std::vector<ParticlePos>& data;

 private:
  std::mutex& m_mut;
};

ParticlePair EvenLayout(const size_t size);
ParticlePair UniformLayout(const size_t size);

class NBody {
 public:
  NBody(const size_t size,
        std::function<ParticlePair(const size_t)> fun = UniformLayout);
  ~NBody();

  void Init();
  void Clean();

  /// Its not this simulations responsibility to calculate latency.
  void Update();

  /// <summary>
  ///  Returns none if no new data is available.
  /// </summary>
  /// <returns></returns>
  MyMutex* GetNewData() {
    std::lock_guard donemut(m_done_mutex);

    if (m_newdata && m_writing_mutex.try_lock()) {
      m_newdata = false;
      return new MyMutex(m_writing_mutex, m_particles_pos);
    }
    return nullptr;
  }

 private:
  std::mutex m_writing_mutex;
  std::mutex m_done_mutex;
  bool m_newdata = true;
  NBodyTimer m_timer;
  std::vector<ParticleData> m_particles_data;
  std::vector<ParticlePos> m_particles_pos;
  friend std::ostream& operator<<(std::ostream& os, const NBody& self) {
    os << "NBody(" << self.m_particles_pos.size() << ")";

    return os;
  }
};
