#include "Octree.h"

struct InsertTabs {
  size_t count;
  InsertTabs(const size_t count) : count(count){};
};

std::ostream &operator<<(std::ostream &os, const InsertTabs &count) {
  for (size_t i = 0; i < count.count; i++) os << "\t";
  return os;
}

struct UserInfo {
  bool printed = false;
};
std::ostream &operator<<(std::ostream &os,
                         NodeWithExtraInfo<UserInfo> *current) {
  if (current->user.printed) return os;

  if (current->parent != nullptr) {
    os << current->parent;
  }
  os << InsertTabs(current->depth) << "Node:"
     << "\n"
     << InsertTabs(current->depth) << "depth: " << current->depth
     << "\n"
     << InsertTabs(current->depth) << "center: " << current->center
     << "\n"
     << InsertTabs(current->depth) << "size: " << current->size
     << "\n"
     << InsertTabs(current->depth) << "other: centerOfMass" <<
     current->inner->m_center_of_mass
     << "\n"
     << InsertTabs(current->depth) << "Mass: "
     << current->inner->m_mass
     << "\n"
     << InsertTabs(current->depth) << "children: "
     << std::endl;

  current->user.printed = true;

  return os;
}
std::ostream &operator<<(std::ostream &os, const Octree &octree) {
  /// Set sizes for Octree Nodes

  os << "Octree: \n{\n";

  octree.DepthFirstTravel<UserInfo>(
      [&os](std::shared_ptr<NodeWithExtraInfo<UserInfo>> current) {
        switch (current->inner->m_data_type) {
          case Parent:
          case Leaf:

            if (current->inner->m_particle != nullptr) {
              os << current;
              os << InsertTabs(current->depth + 1)
                 << "Particle: " << Vec3<float>(*current->inner->m_particle)
                 << std::endl;
            }
            break;
        }
      });

  os << "}" << std::endl;
  os << "center" << Vec3<float>(octree.m_center) << std::endl;
  os << "size" << Vec3<float>(octree.m_size) << std::endl;

  return os;
}