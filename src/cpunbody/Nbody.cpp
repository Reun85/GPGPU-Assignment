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
  for (int i = -num; i <= num; i++) {
    for (int j = -num; j <= num; j++) {
      for (int k = -num; k <= num; k++) {
        particles.emplace_back(
            Particle{glm::vec3(static_cast<float>(i), static_cast<float>(j),
                               static_cast<float>(k)),
                     {},
                     {}});
      }
    }
  }
  BoundingBox bb(particles);
  Octree oc(particles);
  std::cout << oc << std::endl;
  // Node* s = oc.m_root->m_children[0]->m_children[0];
  // std::vector<Node*> stack;
  // stack.push_back(s);
  // while (!stack.empty()) {
  //   Node* current = stack.back();
  //   stack.pop_back();
  //   if (!current->m_is_leaf) {
  //     for (int i = 0; i < 8; i++) {
  //       stack.push_back(current->m_children[i]);
  //     }
  //   } else {
  //     if (current->m_particle != nullptr) {
  //       std::cout << "Particle: "
  //                 << Vec3<float>(current->m_particle->m_position) <<
  //                 std::endl;
  //     }
  //   }
  // }
  return 1;
}
