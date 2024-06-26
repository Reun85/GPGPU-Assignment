#include "SimulationSettings.h"

SimulationSettings::SimulationSettings(int particle_count) : particle_count(particle_count) {
  layout = [](int size) -> ParticleSetDescription { return Galaxy(size, 50000); };
}

bool SimulationSettings::SetSettings(NBody& n) {
  bool has_to_restart = restart;
	if (changed) {
		// TODO: finish this
	changed = false;
	restart = false;
  }
  return has_to_restart;
}