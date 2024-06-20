#pragma once

#include "Vec3.hpp"

static const float default_mass = 5000.f;

// Does not include position!
class ParticleData {
  using Prec = float;
  using Ty_Mass = float;

 public:
  vec3 m_velocity;
  vec3 m_force;

  Ty_Mass m_mass;

  ParticleData(Ty_Mass mass = default_mass,vec3 velocity = vec3(0, 0, 0))
      : m_velocity(velocity), m_mass(mass), m_force(0, 0, 0) {}

  void resetForce() { m_force = vec3(0, 0, 0); }

  friend std::ostream& operator<<(std::ostream& os, const ParticleData& part) {
    os << "particle(" << part.m_velocity << "," << part.m_mass << ")";
    return os;
  }
};
using ParticlePos = vec3;
using ParticlePair =
    std::pair<std::vector<ParticlePos>, std::vector<ParticleData>>;



