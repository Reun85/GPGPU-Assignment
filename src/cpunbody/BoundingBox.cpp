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

BoundingBox::BoundingBox(const std::vector<Particle> &particles) {
  // GPU like implementation
  // 256 groups
  static const size_t workGroupSize = 4;
  const size_t numpergroup = (particles.size()) / (workGroupSize);
  std::array<std::pair<glm::vec3, glm::vec3>, workGroupSize> minmax{};
  for (int i = 0; i < workGroupSize; i++) {
    glm::vec3 min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    glm::vec3 max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    size_t start = i * numpergroup;
    size_t end = std::min(start + numpergroup, particles.size());
    for (int j = start; j < end; j++) {
      min = glm::min(min, particles[j].m_position);
      max = glm::max(max, particles[j].m_position);
    }
    minmax[i] = std::make_pair(min, max);
  }

  glm::vec3 min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
  glm::vec3 max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  for (int i = 0; i < workGroupSize; i++) {
    min = glm::min(min, minmax[i].first);
    max = glm::max(max, minmax[i].second);
  }
  m_min = min;
  m_max = max;
}
