#pragma once

#include "Layout.h"

class SimulationSettingsEditor;
class NBody;
class SimulationSettings {
 public:
  ~SimulationSettings() = default;

 public:
  SimulationSettings(){};

 private:
  SimulationSettings(const int _particle_count,
                     const LayoutResultFunction& _layout,
                     const float _distanceThreshold, const float _eps,
                     const float _gravitation_constant, const int start_depth);
  bool operator==(const SimulationSettings& other) const;

  bool operator!=(const SimulationSettings& other) const {
    return !(*this == other);
  }

 public:
  int particle_count;
  LayoutResultFunction layout;
  bool layoutchanged=false;
  float distanceThreshold;
  float eps;
  // float gravitational_constant = 6.67430e-11f;
  float gravitational_constant;

  int start_depth;
  friend SimulationSettingsEditor;
  friend NBody;
  int position_update_items_per_thread = 16;
  int barneshut_items_per_thread = 4;
  int divide_by_mass_threads = 2048;
  int center_of_mass_items_per_thread = 8;
  int allocatedNodes = ((1uLL << (3 * 5)) * 200uLL * 8uLL);

  int boundingbox_work_group_size = 256;
  float timestep = 0.1f;
};

class SimulationSettingsEditor {
  static constexpr int DEFAULT_PARTICLE_COUNT = 66048;
  static constexpr float DISTANCE_THRESHOLD = 0.3f;
  static constexpr float EPS = 1e-3f;
  static constexpr float GRAVITATION_CONSTANT = 6.67430e-11f;
  static constexpr int START_DEPTH = 5;

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
