#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "Particle.h"

/// AABB Bounding Box
class BoundingBox {
 public:
  BoundingBox(glm::vec3 min, glm::vec3 max) : m_min(min), m_max(max) {}
  BoundingBox(const std::vector<Particle> &particles);

  /// NOTE: May be used later.
  // bool Contains(const Particle &particle) const;
  // bool Intersects(const BoundingBox &other) const;

  glm::vec3 GetMin() const { return m_min; }
  glm::vec3 GetMax() const { return m_max; }
  glm::vec3 GetCenter() const { return (m_min + m_max) / 2.0f; }

 private:
  glm::vec3 m_min;
  glm::vec3 m_max;
};
