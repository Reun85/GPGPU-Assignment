// Utils
#include "Nbody.h"

#include <iostream>

#include "cpunbody/Octree.h"
NBody::NBody(const int size) {}
NBody::~NBody() {}

void NBody::Init() {}
void NBody::Clean() {}

void NBody::Update(const SUpdateInfo&) {}

int test() {
  std::vector<Particle> particles;
  static constexpr int num = 2;
  glm::vec3 offset(5.f, 6.f, 7.f);
  glm::vec3 size(2.f, 1.f, 0.5f);
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
