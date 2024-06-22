// Utils
#include "cpunbody/NBody.h"

#include <cmath>
#include <iostream>
#include <random>

#include "BarnesHut.h"
#include "Octree.h"
#include "Vec3.hpp"
ParticlePair UniformLayout(const size_t size) {
  std::vector<ParticlePos> particles;
  std::vector<ParticleData> particle_data(size);
  particles.reserve(size);

  std::default_random_engine generator;
  std::normal_distribution<float> distribution(0.0, 1.0);

  for (size_t i = 0; i < size; ++i) {
    float x = distribution(generator);
    float y = distribution(generator);
    float z = distribution(generator);
    particles.emplace_back(ParticlePos{vec3(x, y, z)});
  }

  return std::make_pair(particles, particle_data);
}

ParticlePair EvenLayout(const size_t count) {
  std::vector<ParticlePos> particles;
  std::vector<ParticleData> particle_data(count);
  const int num = std::ceil((std::cbrt(static_cast<float>(count)) - 1.) / 2.);
  const size_t res = static_cast<size_t>(std::pow(num * 2 + 1, 3));
  std::cerr << "number of particles:" << res << std::endl;

  particles.reserve(res);
  vec3 offset(0.f, 0.f, 0.f);
  vec3 size(1.f, 1.f, 1.f);
  for (int i = -num; i <= num; i++) {
    for (int j = -num; j <= num; j++) {
      for (int k = -num; k <= num; k++) {
        vec3 pos =
            offset + size * vec3(static_cast<float>(i), static_cast<float>(j),
                                 static_cast<float>(k));
        particles.emplace_back(ParticlePos{pos});
      }
    }
  }
  return std::make_pair(particles, particle_data);
}

NBody::NBody(const size_t size, std::function<ParticlePair(const size_t)> fun) {
  ParticlePair particles = fun(size);

  m_particles_pos = particles.first;
  m_particles_data = particles.second;
  m_particles_pos[0] = vec3(-5.f, -5.f, -5.f);
  m_particles_data[0] = ParticleData(1e+11);
  Init();
}
NBody::~NBody() {}

void NBody::Init() {}
void NBody::Clean() {}

void NBody::Update() {
  Octree oc(m_particles_pos, m_particles_data);

  BarnesHut(m_particles_pos, m_particles_data, oc, m_timer, m_writing_mutex);
  std::lock_guard<std::mutex> done_guard(m_done_mutex);
  m_newdata = true;
}
