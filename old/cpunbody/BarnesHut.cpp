#include "BarnesHut.h"

const size_t ParticlePerGroup = 16;
const float distanceThreshold = 0.2f;
const float eps = 1e-3f;
static constexpr double G = 6.67430e-11f;

void BarnesHut(std::vector<ParticlePos>& particles,
               std::vector<ParticleData>& particles_data, const Octree& oc,
               NBodyTimer& timer, std::mutex& tolock) {
  const size_t numberOfGroups = (particles.size()) / (ParticlePerGroup);

  for (size_t local_id = 0; local_id < numberOfGroups; ++local_id) {
    size_t start = local_id * ParticlePerGroup;
    size_t end = std::min(start + ParticlePerGroup, particles.size());
    for (size_t id = start; id < end; id++) {
      ParticlePos& particle_pos = particles[id];
      ParticleData& particle_data = particles_data[id];
      vec3 force(0.f);

      Node* stack[512];
      int stackSize = 0;
      for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
          stack[stackSize++] = oc.m_root->m_children[i]->m_children[j];

      while (stackSize > 0) {
        assert(stackSize < 128);
        Node* node = stack[--stackSize];

        if (node->m_mass == 0) continue;

        Vec3 delta = node->m_center_of_mass - particle_pos;
        float distance = sqrt(delta.x * delta.x + delta.y * delta.y +
                              delta.z * delta.z + eps * eps);

        float d = node->m_region_size.biggest_component() * 2 / distance;

        if (node->m_data_type == NodeDataType ::Leaf || d < distanceThreshold) {
          float F =
              (G * particle_data.m_mass * node->m_mass) / (distance * distance);
          vec3 add = delta * F / distance;
          force += add;
        } else {
          for (int j = 0; j < 8; ++j) {
            stack[stackSize++] = node->m_children[j];
          }
        }
      }
      particle_data.m_force = force;
    }
  }
  std::cout << std::endl;
  float dt = timer.Tick() * 1.f;
  dt = 0.1f;
  std::cout << "Timer ticked with <" << dt << "> seconds" << std::endl;

  float avgforce = 0;
  std::lock_guard<std::mutex> guard(tolock);
  for (size_t i = 0; i < particles.size(); i++) {
    ParticlePos& particle = particles[i];
    ParticleData& particle_data = particles_data[i];
    particle_data.m_velocity +=
        particle_data.m_force / particle_data.m_mass * dt;
    particle += particle_data.m_velocity * dt;
    avgforce += particle_data.m_force.magnitude();
  }
  std::cout << "Average force applied: " << avgforce / particles.size()
            << std::endl;
}
