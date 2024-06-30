#pragma once

#include "Layout.h"

inline constexpr size_t default_allocated_nodes_size_from_start_depth(
    int start_depth, size_t mult) {
  return ((1uLL << (3 * start_depth)) * mult);
}

class SimulationSettingsEditor;
class NBody;
struct SimulationSettings {
 public:
  ~SimulationSettings() = default;

 public:
  SimulationSettings(){};

  SimulationSettings(const int _particle_count,
                     const LayoutResultFunction& _layout,
                     const float _distance_threshold, const float _eps,
                     const float _gravitational_constant,
                     const int _start_depth);

  bool operator==(const SimulationSettings& other) const;

  bool operator!=(const SimulationSettings& other) const {
    return !(*this == other);
  }

 public:
  int particle_count;
  LayoutResultFunction layout;
  bool layoutchanged = false;
  float distance_threshold;
  float eps;
  // float gravitational_constant = 6.67430e-11f;
  float gravitational_constant;

  int start_depth;
  friend SimulationSettingsEditor;
  friend NBody;
  int position_update_items_per_thread;
  int barneshut_items_per_thread;
  int divide_by_mass_threads;
  int center_of_mass_items_per_thread;
  int allocatedNodes;

  int boundingbox_work_group_size;
  float max_timestep;
};

class SimulationSettingsEditor {
  static constexpr int DEFAULT_PARTICLE_COUNT = 100;
  static constexpr float DEFAULT_DISTANCE_THRESHOLD = 0.3f;
  static constexpr float DEFAULT_EPS = 1e-3f;
  static constexpr float DEFAULT_GRAVITATION_CONSTANT = 6.67430e-11f;
  static constexpr int DEFAULT_START_DEPTH = 5;
  static constexpr float DEFUALT_MAX_TIMESTEP = 0.1f;
  static constexpr int DEFAULT_BOUNDING_BOX_WORK_GROUP_SIZE = 256;
  static constexpr size_t DEFAULT_ALLOCATED_NODES_COUNT =
      default_allocated_nodes_size_from_start_depth(DEFAULT_START_DEPTH,
                                                    200 * 8);
  static constexpr int DEFAULT_CENTER_OF_MASS_ITEMS_PER_THREAD = 8;
  static constexpr int DEFAULT_DIVIDE_BY_MASS_THREADS = 2048;
  static constexpr int DEFAULT_BARNES_HUT_ITEMS_PER_THREAD = 4;
  static constexpr int DEFAULT_POSITION_UPDATE_ITEMS_PER_THREAD = 16;
  static constexpr LayoutSelector::SimulationMode DEFAULT_LAYOUT =
      LayoutSelector::SimulationMode::Galaxy;

 public:
  enum State { ResetNoChanges, ResetChanges, Off, On, NoChanges };
  SimulationSettingsEditor();
  SimulationSettings& GetCurrSettings() { return curr; };
  LayoutSelector& GetCurrLayout() { return currlayout; };

  // Returns true if a change is requested.
  State Render();
  void Apply();

  void SetCrashed(std::exception ex);

 private:
  bool has_anything_changed() const;
  bool ison = false;
  LayoutSelector currlayout;
  SimulationSettings curr;

  std::optional<std::exception> crash;

  // So we can have nice imgui buttons
  LayoutSelector prevlayout;
  SimulationSettings prev;
};
