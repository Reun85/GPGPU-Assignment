#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>

// std
#include <vector>

// Utils
#include "Particle.h"
#include "SUpdateInfo.h"

struct DefaultLayout {
 public:
  static std::vector<Particle> Generate(const size_t size);
};

class NBody {
 public:
  template <typename T = DefaultLayout>
  NBody(const size_t size);
  ~NBody();

  void Init();
  void Clean();

  /// Its not this simulations responsibility to calculate latency.
  void Update(const SUpdateInfo &);

 private:
  std::vector<Particle> m_particles;
};

template <typename T>
NBody::NBody(const size_t size) : m_particles(T::Generate(size)) {
  Init();
}
int testNBody();
