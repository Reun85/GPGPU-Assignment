#include "BoundingBox.h"

#include <array>

// BoundingBox::BoundingBox(const std::vector<Particle> &particles) {
//   // CPU like implementation
//   min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
//   max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
//   for (const Particle &particle : particles) {
//     min = glm::min(min, particle.m_position);
//     max = glm::max(max, particle.m_position);
//   }
// }

BoundingBox::BoundingBox(const std::vector<ParticlePos> &particles) {
  Recalculate(particles);
}

void BoundingBox::Recalculate(const std::vector<ParticlePos> &particles) {
  // GPU like implementation
  // 256 groups
  static const size_t workGroupSize = 4;
  const size_t numpergroup = (particles.size()) / (workGroupSize);
  std::array<std::pair<vec3, vec3>, workGroupSize> minmax{};
  for (int i = 0; i < workGroupSize; i++) {
    vec3 min = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    vec3 max = vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    size_t start = i * numpergroup;
    size_t end = std::min(start + numpergroup, particles.size());
    for (size_t j = start; j < end; j++) {
      min = vec3::min(min, particles[j]);
      max = vec3::max(max, particles[j]);
    }
    minmax[i] = std::make_pair(min, max);
  }

  vec3 min = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
  vec3 max = vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  for (int i = 0; i < workGroupSize; i++) {
    min = vec3::min(min, minmax[i].first);
    max = vec3::max(max, minmax[i].second);
  }
  m_min = min;
  m_max = max;
}

void BoundingBox::AdjustToFitAll() {
  m_min -= BoundingBox::EPS;
  m_max += BoundingBox::EPS;
}