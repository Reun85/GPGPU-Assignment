#pragma once

#include <glm/glm.hpp>

struct Particle {
  glm::vec3 m_position;
  glm::vec3 m_prevvelocity;
  //
  glm::vec3 m_acceleration;
};
