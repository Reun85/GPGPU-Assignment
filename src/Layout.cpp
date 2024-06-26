#include "Layout.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <random>
ParticleSetDescription Uniform::Generate(const int size,
                                         const float default_mass) {
  std::vector<cl_float3> particles;
  cl_float3 empt;
  empt.x = 0;
  empt.y = 0;
  empt.z = 0;
  ParticleData def_data{empt, empt};
  std::vector<ParticleData> particle_data(size, def_data);
  particles.reserve(size);

  std::default_random_engine generator;
  std::normal_distribution<float> distribution(0.0, 1.0);

  for (size_t i = 0; i < size; ++i) {
    float x = distribution(generator);
    float y = distribution(generator);
    float z = distribution(generator);
    cl_float3 t;
    t.x = x;
    t.y = y;
    t.z = z;
    t.w = default_mass;

    particles.push_back(t);
  }

  return std::make_pair(particles, particle_data);
}

const float PI = 3.14159265359;

void GalaxySinglePointGen(std::normal_distribution<float> &distribution,
                          std::default_random_engine &generator,
                          const float &default_mass, float r,
                          const glm::vec3 &g2_center,
                          const glm::vec3 &g2_velocity,
                          std::vector<cl_float4> &particles,
                          std::vector<ParticleData> &particle_data) {
  float rng1 = distribution(generator) * 2.f * PI;
  float rng2 = distribution(generator);
  float rng3 = distribution(generator);
  float rng4 = distribution(generator);
  rng4 = pow((rng4 + 1.0f) / 2.0f, 1.0f) * default_mass * 0.25f;
  glm::vec3 _pos =
      glm::vec3(cos(rng1) * rng2 * 1.0f, rng3 / 20.0f, sin(rng1) * rng2 * 1.0f);
  glm::mat4 rotationMatrix =
      glm::rotate(glm::mat4(1.0f), r, glm::vec3(1.0f, 0.0f, 0.0f));
  _pos = glm::vec3(rotationMatrix * glm::vec4(_pos, 1.0f)) + g2_center;

  glm::vec3 tang_vel = glm::vec3(glm::normalize(
      glm::cross(glm::vec3(0, 1, 0), glm::vec3(_pos) - g2_center)));
  float dis = glm::distance(glm::vec3(_pos), g2_center);
  glm::vec3 rnd_vel = tang_vel * (dis)*25.f;
  rnd_vel /= 100;

  rnd_vel += g2_velocity;

  particles.push_back({{_pos.x, _pos.y, _pos.z, rng4}});
  particle_data.push_back({{rnd_vel.x, rnd_vel.y, rnd_vel.z}, {{0, 0, 0}}});
}

ParticleSetDescription GalaxiesClashing::Generate(const int size,
                                                  const float default_mass) {
  const glm::vec3 g1_center = glm::vec3(3, 3, 3);
  const glm::vec3 g2_center = -g1_center;
  // Pulled towards this =>
  const glm::vec3 g_center = glm::vec3(0, 0, 0);

  const glm::vec3 g1_velocity = (g2_center - g1_center) * 0.01f;
  const glm::vec3 g2_velocity = glm::vec3(0);

  std::vector<cl_float3> particles;
  std::vector<ParticleData> particle_data;
  particles.reserve(size);
  particle_data.reserve(size);

  std::default_random_engine generator;
  std::normal_distribution<float> distribution(0.0, 1.0);

  float r = distribution(generator) * PI;
  for (size_t i = 0; i < size / 2; ++i) {
    GalaxySinglePointGen(distribution, generator, default_mass, r, g1_center,
                         g1_velocity, particles, particle_data);
  }
  r = distribution(generator) * PI;
  for (size_t i = 0; i < size - size / 2; ++i) {
    GalaxySinglePointGen(distribution, generator, default_mass, r, g2_center,
                         g2_velocity, particles, particle_data);
  }
  return std::make_pair(particles, particle_data);
}

ParticleSetDescription Galaxy::Generate(const int size,
                                        const float default_mass,const glm::vec3 g_center,const glm::vec3 g_velocity) {

  std::vector<cl_float3> particles;
  std::vector<ParticleData> particle_data;
  particles.reserve(size);
  particle_data.reserve(size);

  std::default_random_engine generator;
  std::normal_distribution<float> distribution(0.0, 1.0);

  float r = distribution(generator) * PI;
  for (size_t i = 0; i < size; ++i) {
    GalaxySinglePointGen(distribution, generator, default_mass, r, g_center,
                         g_velocity, particles, particle_data);
  }
  return std::make_pair(particles, particle_data);
}