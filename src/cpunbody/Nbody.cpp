// Utils
#include "Nbody.h"

#include <chrono>
#include <iostream>

#include "cpunbody/Octree.h"
NBody::NBody(const int size) {}
NBody::~NBody() {}

void NBody::Init() {}
void NBody::Clean() {}

void NBody::Update(const SUpdateInfo&) {}

int testNBody() {
  std::vector<Particle> particles;
  static constexpr int num = 4;
  static const size_t res = std::pow(num * 2 + 1, 3);
  auto timestmp1 = std::chrono::high_resolution_clock::now();
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
  auto timestmp2 = std::chrono::high_resolution_clock::now();
  BoundingBox bb(particles);
  auto timestmp3 = std::chrono::high_resolution_clock::now();
  Octree oc(particles, timestmp3);
  auto timestmp4 = std::chrono::high_resolution_clock::now();
  std::cout << oc << std::endl;
  auto timestmp5 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> elapsed1 = timestmp3 - timestmp2;
  std::chrono::duration<double, std::milli> elapsed2 = timestmp4 - timestmp3;
  std::chrono::duration<double, std::milli> elapsed3 = timestmp5 - timestmp4;
  std::cerr << "Time to generate: " << elapsed1.count()
            << "\nTime to parse: " << elapsed2.count()
            << "\nTime to print: " << elapsed3.count()
            << "\nBoundingBox Calculate: "
            << std::chrono::duration<double, std::milli>(timestmp2 - timestmp1)
                   .count()
            << std::endl;

  return 1;
}
