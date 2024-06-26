#pragma once

#include "NBody.h"

#include <glm/glm.hpp>

class Galaxy {

  glm::vec3 g_center = glm::vec3(0, 0, 0);
  glm::vec3 g_velocity = glm::vec3(0, 0, 0);
  float default_mass = 5000;
 static        ParticleSetDescription Generate(const int size, const float default_mass,const glm::vec3 g_center,const glm::vec3 g_velocity);
  std::function<ParticleSetDescription(const int size) > GetLayout() const {
    float mass = default_mass;
    glm::vec3 center = g_center;
    glm::vec3 velocity = g_velocity;
    return [mass,center,velocity](const int size) -> ParticleSetDescription {
      return Galaxy::Generate(size, mass,center,velocity);
    };
  };
};

class GalaxiesClashing{
  float default_mass = 5000;

  static ParticleSetDescription Generate(const int size, const float);
	std::function<ParticleSetDescription(const int size) > GetLayout() const;

};
class Uniform{
  float default_mass = 5000;

static ParticleSetDescription Generate(const int size, const float);
	std::function<ParticleSetDescription(const int size) > GetLayout() const;
};

