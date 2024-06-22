#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "NBody.h"
#include "Octree.h"
#include "Particle.h"

void BarnesHut(std::vector<ParticlePos>& particles,
               std::vector<ParticleData>& particle_data, const Octree& oc,
               NBodyTimer& timer, std::mutex& tolock);
