#pragma once

#include "Layout.h"
#include "NBody.h"

class SimulationSettingsEditor;
class NBody;
class SimulationSettings {
 public:
  ~SimulationSettings() = default;

 private:
  SimulationSettings(const int _particle_count,
                     const LayoutResultFunction& _layout,
                     const float _distanceThreshold, const float _eps,
                     const float _gravitation_constant, const int start_depth);

 public:
  int particle_count;
  LayoutResultFunction layout;
  float distanceThreshold = 0.1f;
  float eps = 1e-3f;
  // float gravitational_constant = 6.67430e-11f;
  float gravitational_constant = 6.67430e-4f;

  int start_depth = 4;
  friend SimulationSettingsEditor;
  friend NBody;
};

class SimulationSettingsEditor {
  static constexpr int DEFAULT_PARTICLE_COUNT = 1e+6;
  static constexpr float DISTANCE_THRESHOLD = 0.1f;
  static constexpr float EPS = 1e-3f;
  static constexpr float GRAVITATION_CONSTANT = 6.67430e-4f;
  static constexpr int START_DEPTH = 4;

 public:
  SimulationSettingsEditor();
  SimulationSettings& GetCurrSettings() { return curr; };
  LayoutSelector& GetCurrLayout() { return currlayout; };

  // Returns true if a change is requested.
  bool Render();
  void Save();

 private:
  bool has_anything_changed() const;
  LayoutSelector currlayout;
  SimulationSettings curr;

  // So we can have nice imgui buttons
  LayoutSelector prevlayout;
  SimulationSettings prev;
};
