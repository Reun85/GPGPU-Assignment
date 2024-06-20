// Utils
#include "Nbody.h"

#include <cmath>
#include <iostream>
#include <random>

#include "cpunbody/Octree.h"

std::vector<Particle> DefaultLayout::Generate(const size_t size) {
  std::vector<Particle> particles;
  particles.reserve(size);

  std::default_random_engine generator;
  std::normal_distribution<float> distribution(0.0, 2.0);

  for (size_t i = 0; i < size; ++i) {
    float x = distribution(generator);
    float y = distribution(generator);
    float z = distribution(generator);
    particles.emplace_back(
        Particle{glm::vec3(x, y, z), glm::vec3(0), glm::vec3(0)});
  }

  return particles;
}

std::vector<Particle> EvenLayout::Generate(const size_t count) {
  std::vector<Particle> particles;
  const int num = std::ceil((std::cbrt(static_cast<float>(count)) - 1.) / 2.);
  const size_t res = std::pow(num * 2 + 1, 3);
  std::cerr << "number of particles:" << res << std::endl;

  particles.reserve(res);
  glm::vec3 offset(0.f, 0.f, 0.f);
  glm::vec3 size(1.f, 1.f, 1.f);
  for (int i = -num; i <= num; i++) {
    for (int j = -num; j <= num; j++) {
      for (int k = -num; k <= num; k++) {
        glm::vec3 pos = offset + size * glm::vec3(static_cast<float>(i),
                                                  static_cast<float>(j),
                                                  static_cast<float>(k));
        particles.emplace_back(Particle{pos, {}, {}});
      }
    }
  }
  return particles;
}

NBody::~NBody() {}

void NBody::Init() {
  BoundingBox bb(m_particles);
  Octree oc(m_particles);
  std::cout << oc << std::endl;
}
void NBody::Clean() {}

void NBody::Update(const SUpdateInfo&) {}

int testNBody() {
  std::vector<Particle> particles;
  static constexpr int num = 4;
  static const size_t res = std::pow(num * 2 + 1, 3);
  std::cerr << "number of particles:" << res << std::endl;

  particles.reserve(res);
  glm::vec3 offset(0.f, 0.f, 0.f);
  glm::vec3 size(1.f, 1.f, 1.f);
  for (int i = -num; i <= num; i++) {
    for (int j = -num; j <= num; j++) {
      for (int k = -num; k <= num; k++) {
        glm::vec3 pos = offset + size * glm::vec3(static_cast<float>(i),
                                                  static_cast<float>(j),
                                                  static_cast<float>(k));
        particles.emplace_back(Particle{pos, {}, {}});
      }
    }
  }
  BoundingBox bb(particles);
  Octree oc(particles);
  std::cout << oc << std::endl;

  return 1;
}
