#pragma once

#include "NBody.h"
#include "Layout.h"


using Layout = std::function<ParticleSetDescription(const size_t)>;

class SimulationSettings {
  static constexpr int DEFAULT_PARTICLE_COUNT = 1e+6;
 public:
  SimulationSettings(int particle_count= DEFAULT_PARTICLE_COUNT);
  ~SimulationSettings() = default;

  void SetParticleCount(const int count) { particle_count = count; }
  void SetBarnesHutThreshold(const float threshold) { distanceThreshold = threshold; }
  void SetBarnesHutEps(const float _eps) { eps = _eps; }
  void SetGravitationalConstant(const float constant) {
    gravitational_constant = constant;
  }
  void SetLayout(const Layout _layout) { layout = _layout; }
  void SetStartDepth(const int depth) { start_depth = depth; }

  int GetParticleCount() const { return particle_count; }
  float GetBarnesHutThreshold() const { return distanceThreshold; }
  float GetBarnesHutEps() const { return eps; }
  float GetGravitationalConstant() const { return gravitational_constant; }
  int GetStartDepth() const { return start_depth; }
  Layout GetLayout() const { return layout; }


  bool SetSettings(NBody& n);

 private:
  bool changed = false;
  bool restart = false;


  int particle_count;
  Layout layout;


  int particle_count;
  std::function<ParticleSetDescription(const size_t)> generating_function;
  float distanceThreshold = 0.1f;
  float eps = 1e-3f;
  //float gravitational_constant = 6.67430e-11f;
  float gravitational_constant = 6.67430e-4f;

  int start_depth = 4;
};