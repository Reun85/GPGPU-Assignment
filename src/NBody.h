#pragma once

#include <CLPreComp.h>
#include <GL/glew.h>

#include <chrono>
#include <cstddef>
#include <exception>
#include <functional>
#include <mutex>
#include <vector>

#include "Communication.hpp"
#include "ParticleDescription.h"

class NBodyTimer {
 public:
  float Tick();
  NBodyTimer();

 private:
  std::chrono::high_resolution_clock::time_point prev;
};

class NBody {
 public:
  NBody(const SimulationSettings& s);
  ~NBody();

  // Initialise OpenCL, attach OpenGL Buffers
  bool InitCL(const GLuint& VBOIndex);

  /// Generates the particles and moves them to the GPU.
  void Start();

  // Returns whether it has should be restarted
  // Giving nullopt will apply the current settings again.
  bool ChangeSettings(const std::optional<SimulationSettings> s);

  void Clean();

  // Update the simulationstate
  void Calculate();

  /// Attempts to write to the given OpenGL buffer
  bool TryAndWriteData();

  // Writes simulation results to the given buffer
  void UpdateCommunication(Communication& comm);

 private:
	 // Tests
  void doTesting();

  //          ╭─────────────────────────────────────────────────────────╮
  //          │                  Simulation Constants                   │
  //          ╰─────────────────────────────────────────────────────────╯
  SimulationSettings settings;

  //          ╭─────────────────────────────────────────────────────────╮
  //          │                  Writing communication                  │
  //          ╰─────────────────────────────────────────────────────────╯

  SimulationData simulation_results;
  std::mutex m_writing_mutex;
  std::mutex m_done_mutex;
  bool m_newdata = true;

  //          ╭─────────────────────────────────────────────────────────╮
  //          │                           CL                            │
  //          ╰─────────────────────────────────────────────────────────╯
  GLuint VBO;

  NBodyTimer timer;

  cl::Kernel boundingbox;
  cl::Kernel boundingboxstage2;

  cl::Kernel createOctree;
  cl::Kernel initOctree;
  cl::Kernel buildOctree;

  cl::Kernel centerofMass;
  cl::Kernel DivideByMass;

  cl::Kernel barneshut;
  cl::Kernel positionupdate;

  cl::Context context;
  // Simulation command_queue
  cl::CommandQueue command_queue;
  //// Copy command_queue
  cl::CommandQueue copy_command_queue;
  cl::Program program;

  // CL buffers

  // Cannot change size
  cl::BufferGL openGLparticlepos;
  std::vector<cl::Memory> GLbuffers;
  cl::Buffer itrBuffer;
  cl::Buffer globalMinBuffer;
  cl::Buffer globalMaxBuffer;
  // Can change
  cl::Buffer particlepos;
  cl::Buffer particledata;
  cl::Buffer Nodes;
  cl::Buffer minValuesBuffer;
  cl::Buffer maxValuesBuffer;
};
