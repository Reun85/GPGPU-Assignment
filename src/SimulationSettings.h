#pragma once

#include <optional>

#include "Layout.h"

inline constexpr size_t default_allocated_nodes_size_from_start_depth(
    int start_depth, size_t mult) {
  return ((1uLL << (3 * start_depth)) * mult);
}

class SimulationSettingsEditor;
class NBody;
struct SimulationSettings {
 public:
  // Will hold invalid data.
  size_t GetVRAMFromSettings() const;
  SimulationSettings() = default;

  SimulationSettings(
      const int _particle_count, const LayoutResultFunction _layout,
      const float _distance_threshold, const float _eps,
      const float _gravitational_constant, const int _start_depth,
      const int _min_enter_depth, const int _max_depth,
      const int _position_update_items_per_thread,
      const int _barneshut_items_per_thread, const int _divide_by_mass_threads,
      const int _center_of_mass_items_per_thread, const int _allocatedNodes,
      const int _boundingbox_work_group_size, const float _max_timestep,
      const int _barneshut_stack_size, const int _build_octree_stack_size)
      : particle_count(_particle_count),
        layout(_layout),
        distance_threshold(_distance_threshold),
        eps(_eps),
        gravitational_constant(_gravitational_constant),
        start_depth(_start_depth),
        min_enter_depth(_min_enter_depth),
        max_depth(_max_depth),
        position_update_items_per_thread(_position_update_items_per_thread),
        barneshut_items_per_thread(_barneshut_items_per_thread),
        divide_by_mass_threads(_divide_by_mass_threads),
        center_of_mass_items_per_thread(_center_of_mass_items_per_thread),
        allocatedNodes(_allocatedNodes),
        boundingbox_work_group_size(_boundingbox_work_group_size),
        max_timestep(_max_timestep),
        barneshut_stack_size(_barneshut_stack_size),
        build_octree_stack_size(_build_octree_stack_size) {}

  bool operator==(const SimulationSettings& other) const;

  bool operator!=(const SimulationSettings& other) const {
    return !(*this == other);
  }

 public:
  // Requires restart
  int particle_count;
  LayoutResultFunction layout;
  bool layoutchanged = false;

  // Requires recompile
  // Should be defined on the command line
  int barneshut_stack_size;
  int build_octree_stack_size;
  int boundingbox_work_group_size;

  // Can be changed anytime
  float distance_threshold;
  float eps;
  float gravitational_constant;

  int start_depth;
  int min_enter_depth;
  int max_depth;
  int position_update_items_per_thread;
  int barneshut_items_per_thread;
  int divide_by_mass_threads;
  int center_of_mass_items_per_thread;
  int allocatedNodes;

  float max_timestep;

  friend SimulationSettingsEditor;
  friend NBody;
};

static constexpr int DEFAULT_BARNESHUT_STACK_SIZE = 512;
static constexpr int DEFAULT_BUILD_OCTREE_STACK_SIZE = 8;
static constexpr int DEFAULT_BOUNDING_BOX_WORK_GROUP_SIZE = 256;

static constexpr int DEFAULT_PARTICLE_COUNT = 100;
static constexpr float DEFAULT_DISTANCE_THRESHOLD = 0.3f;
static constexpr float DEFAULT_EPS = 1e-3f;
static constexpr float DEFAULT_GRAVITATION_CONSTANT = 6.67430e-11f;
static constexpr int DEFAULT_START_DEPTH = 5;
static constexpr int DEFAULT_MIN_ENTER_DEPTH = 13;
static constexpr int DEFAULT_MAX_DEPTH = 18;
static constexpr float DEFAULT_MAX_TIME_STEP = 0.1f;
static constexpr size_t DEFAULT_ALLOCATED_NODES_COUNT =
    default_allocated_nodes_size_from_start_depth(DEFAULT_START_DEPTH, 50 * 8);
static constexpr int DEFAULT_CENTER_OF_MASS_ITEMS_PER_THREAD = 8;
static constexpr int DEFAULT_DIVIDE_BY_MASS_THREADS = 2048;
static constexpr int DEFAULT_BARNES_HUT_ITEMS_PER_THREAD = 4;
static constexpr int DEFAULT_POSITION_UPDATE_ITEMS_PER_THREAD = 16;
static constexpr LayoutSelector::SimulationMode DEFAULT_LAYOUT =
    LayoutSelector::SimulationMode::Galaxy;

class SimulationSettingsEditor {
 public:
  struct Command {
    bool on = false;
    bool regenerate_particles = false;
    bool apply_changes = false;
  };

  SimulationSettingsEditor();
  SimulationSettings& GetCurrSettings() { return curr; };
  LayoutSelector& GetCurrLayout() { return currlayout; };

  std::optional<Command> RenderAndHandleUserInput();

  void SetCrashed(CustomCLError ex);

 private:
  void Apply();
  bool has_anything_changed() const;
  bool ison = false;
  LayoutSelector currlayout;
  SimulationSettings curr;

  std::optional<CustomCLError> crash;

  // So we can have nice imgui buttons
  LayoutSelector prevlayout;
  std::optional<SimulationSettings> prev;
};
