#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "Particle.h"

/// AABB Bounding Box
class BoundingBox {
 public:
  BoundingBox() = default;
  BoundingBox(vec3 min, vec3 max) : m_min(min), m_max(max) {}
  BoundingBox(const std::vector<ParticlePos> &particles);

  void Recalculate(const std::vector<ParticlePos> &particles);

  /// NOTE: May be used later.
  bool Contains(const ParticlePos &particle) const;
  /// NOTE: May be used later.
  bool Intersects(const BoundingBox &other) const;

  vec3 GetMin() const { return m_min; }
  vec3 GetMax() const { return m_max; }
  vec3 GetCenter() const { return (m_min + m_max) / 2.0f; }
  /// Adds a small epsilon to ensure all values are within and not on the
  /// border.
  void AdjustToFitAll();
  const float EPS = 0.001f;

 private:
  vec3 m_min;
  vec3 m_max;
};
