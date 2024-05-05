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

class NBody {
 public:
  NBody(const int size);
  ~NBody();

  void Init();
  void Clean();

  /// Its not this simulations responsibility to calculate latency.
  void Update(const SUpdateInfo &);

 private:
  std::vector<Particle> m_particles;
};

int testNBody();
