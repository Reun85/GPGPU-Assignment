#include "NBody.h"
// #define DEBUG

#include <GL/glew.h>
#include <SDL2/SDL.h>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <GL/glx.h>
#endif

#include <iostream>
#include <vector>
bool ParticleData::operator==(const ParticleData& rhs) {
  return velocity.x == rhs.velocity.x && velocity.y == rhs.velocity.y &&
         velocity.z == rhs.velocity.z && force.x == rhs.force.x &&
         force.y == rhs.force.y && force.z == rhs.force.z;
}
float NBodyTimer::Tick() {
  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> duration = now - prev;
  prev = now;
  return duration.count();
}
NBodyTimer::NBodyTimer() : prev(std::chrono::high_resolution_clock::now()) {}

constexpr size_t global_work_size_from_item_per_thread(
    const size_t total_work_size, const size_t items_per_thread) {
  return (total_work_size + items_per_thread - 1) / items_per_thread;
}

constexpr size_t global_work_size_from_work_groups(
    const size_t total_work_size, const size_t work_group_size) {
  return ((total_work_size + work_group_size - 1) / work_group_size) *
         work_group_size;
}

std::ostream& operator<<(std::ostream& os, const cl_float3& f) {
  os << "(" << f.s[0] << ", " << f.s[1] << ", " << f.s[2] << ")";
  return os;
}
std::ostream& operator<<(std::ostream& os, const ParticleData& pd) {
  os << "ParticleData: {" << std::endl;
  os << "Velocity: " << pd.velocity << std::endl;
  os << "Force: " << pd.force << std::endl;
  os << "}" << std::endl;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Node& n) {
  os << "Node: {" << std::endl;
  os << "Center of mass: " << n.center_of_mass.x << " " << n.center_of_mass.y
     << " " << n.center_of_mass.z << std::endl;
  os << "Mass: " << n.center_of_mass.w << std::endl;
  os << "Region size: " << n.region_size << std::endl;
  os << "Children: [ ";
  for (int i = 0; i < 8; i++) {
    os << n.children[i] << " ";
  }
  os << "]" << std::endl;
  os << "Type: ";
  switch (n.isLeaf) {
    case 2:
      os << "Leaf";
      break;
    case 1:
      os << "Parent";
      break;
    default:
      os << "Empty";
      break;
  }
  os << std::endl << "}" << std::endl;
  return os;
}

#include <fstream>
#include <iostream>
#include <mutex>
#include <oclutils.hpp>
#include <random>
#include <vector>

using namespace cl;
NBody::NBody(const SimulationSettings& s) : settings(s), simulation_results() {}
NBody::~NBody() {}

bool NBody::InitCL(const GLuint& VBOIndex) {
  try {
    ///////////////////////////
    // Initialize OpenCL API //
    ///////////////////////////

    std::vector<cl::Platform> platforms;
    Platform::get(&platforms);

    // Try to get the sharing platform!
    bool create_context_success = false;
    for (auto platform : platforms) {
      // cl_context_properties contextProperties[] = {
      //     CL_CONTEXT_PLATFORM,
      //     (cl_context_properties)(platform)(),
      //     CL_GL_CONTEXT_KHR,
      //     (cl_context_properties)SDL_GL_GetCurrentContext(),
      //     0};

      cl_context_properties contextProperties[] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)(platform)(),
        CL_GL_CONTEXT_KHR,
        (cl_context_properties)SDL_GL_GetCurrentContext(),
#if defined(_WIN32)
        CL_WGL_HDC_KHR,
        (cl_context_properties)wglGetCurrentDC(),
#elif defined(__linux__)
        CL_GLX_DISPLAY_KHR,
        (cl_context_properties)glXGetCurrentDisplay(),
#elif defined(__APPLE__)
        CL_CGL_SHAREGROUP_KHR,
        (cl_context_properties)CGLGetShareGroup(CGLGetCurrentContext()),
#endif
        0
      };

      // Create Context
      try {
        context = cl::Context(CL_DEVICE_TYPE_GPU, contextProperties);
        create_context_success = true;
        break;
      } catch (Error error) {
        oclPrintError(error);
      }
    }

    if (!create_context_success)
      throw cl::Error(CL_INVALID_CONTEXT,
                      "Failed to create CL/GL shared context");

    // Create Command Queue
    cl::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    command_queue =
        cl::CommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);
    copy_command_queue = cl::CommandQueue(context, devices[0]);

    /////////////////////////////////
    // Load, then build the kernel //
    /////////////////////////////////

    // Read source file
    std::ifstream sourceFile("openclkernels.cl");
    std::string sourceCode(std::istreambuf_iterator<char>(sourceFile),
                           (std::istreambuf_iterator<char>()));

    cl::Program::Sources source;
    source.push_back({sourceCode.c_str(), sourceCode.length()});

    // Make program of the source code in the context
    program = cl::Program(context, source);
    try {
      program.build(devices);
    } catch (cl::Error error) {
      std::cout << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0])
                << std::endl;

      throw error;
    }
    // Generate Buffers

    // Make kernel
    boundingbox = cl::Kernel(program, "BoundingBoxStage1");
    boundingboxstage2 = cl::Kernel(program, "BoundingBoxStage2");
    createOctree = cl::Kernel(program, "CreateOctree");
    initOctree = cl::Kernel(program, "InitOctree");
    buildOctree = cl::Kernel(program, "BuildOctree");
    DivideByMass = cl::Kernel(program, "DivideCentersByMass");
    centerofMass = cl::Kernel(program, "CalculateCenterOfMass");
    barneshut = cl::Kernel(program, "BarnesHut");
    positionupdate = cl::Kernel(program, "AddForces");
    VBO = VBOIndex;

    ChangeSettings(std::nullopt);

  } catch (cl::Error error) {
    std::cout << error.what() << "(" << oclErrorString(error.err()) << ")"
              << std::endl;
    return false;
  }

  return true;
}

bool NBody::ChangeSettings(std::optional<SimulationSettings> s) {
  bool recreate_buffers =
      !s.has_value() || settings.allocatedNodes != s->allocatedNodes ||
      settings.particle_count != s->particle_count ||
      settings.boundingbox_work_group_size != s->boundingbox_work_group_size;
  bool requires_restart = recreate_buffers ||
                          settings.start_depth != s->start_depth ||
                          s->layoutchanged;
  if (s.has_value()) settings = *s;
  if (recreate_buffers) {
    std::lock_guard lock(m_writing_mutex);
    if (s.has_value()) {
      // Clear the buffers.
      Nodes = cl::Buffer();
      particledata = cl::Buffer();
      particlepos = cl::Buffer();
      minValuesBuffer = cl::Buffer();
      maxValuesBuffer = cl::Buffer();

      globalMinBuffer = cl::Buffer();
      globalMaxBuffer = cl::Buffer();
      itrBuffer = cl::Buffer();
      openGLparticlepos = cl::BufferGL();
      GLbuffers.clear();
    }
    Nodes = cl::Buffer(context, CL_MEM_READ_WRITE,
                       sizeof(Node) * settings.allocatedNodes);
    particledata = cl::Buffer(context, CL_MEM_READ_WRITE,
                              sizeof(ParticleData) * settings.particle_count);

    particlepos = cl::Buffer(context, CL_MEM_READ_WRITE,
                             sizeof(cl_float4) * settings.particle_count);

    // Intermediate buffers
    minValuesBuffer = cl::Buffer(
        context, CL_MEM_READ_WRITE,
        sizeof(cl_float3) * settings.boundingbox_work_group_size, NULL);
    maxValuesBuffer = cl::Buffer(
        context, CL_MEM_READ_WRITE,
        sizeof(cl_float3) * settings.boundingbox_work_group_size, NULL);
    // This generates the large buffers
    globalMinBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(cl_float3));
    globalMaxBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(cl_float3));
    itrBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(cl_int));
    openGLparticlepos = cl::BufferGL(context, CL_MEM_WRITE_ONLY, VBO);
    GLbuffers.push_back(openGLparticlepos);
  }

  boundingbox.setArg(0, particlepos);
  boundingbox.setArg(1, minValuesBuffer);
  boundingbox.setArg(2, maxValuesBuffer);
  boundingbox.setArg(3, settings.particle_count);
  boundingbox.setArg(4, settings.boundingbox_work_group_size);

  boundingboxstage2.setArg(0, minValuesBuffer);
  boundingboxstage2.setArg(1, maxValuesBuffer);
  boundingboxstage2.setArg(2, globalMinBuffer);
  boundingboxstage2.setArg(3, globalMaxBuffer);
  boundingboxstage2.setArg(4, settings.boundingbox_work_group_size);

  createOctree.setArg(0, Nodes);
  createOctree.setArg(1, settings.start_depth);

  initOctree.setArg(0, Nodes);
  initOctree.setArg(1, settings.start_depth);
  initOctree.setArg(2, globalMinBuffer);
  initOctree.setArg(3, globalMaxBuffer);
  initOctree.setArg(4, itrBuffer);
  initOctree.setArg(5, settings.allocatedNodes);

  buildOctree.setArg(0, particlepos);
  buildOctree.setArg(1, Nodes);
  buildOctree.setArg(2, globalMinBuffer);
  buildOctree.setArg(3, globalMaxBuffer);
  buildOctree.setArg(4, settings.particle_count);
  buildOctree.setArg(5, settings.start_depth);
  buildOctree.setArg(6, itrBuffer);

  DivideByMass.setArg(0, Nodes);
  DivideByMass.setArg(1, itrBuffer);

  centerofMass.setArg(0, Nodes);
  centerofMass.setArg(1, settings.start_depth);
  centerofMass.setArg(2, itrBuffer);

  barneshut.setArg(0, particlepos);
  barneshut.setArg(1, particledata);
  barneshut.setArg(2, Nodes);
  barneshut.setArg(3, settings.particle_count);
  barneshut.setArg(4, settings.distanceThreshold);
  barneshut.setArg(5, settings.eps);
  barneshut.setArg(6, settings.gravitational_constant);
  barneshut.setArg(7, settings.barneshut_items_per_thread);

  positionupdate.setArg(0, particlepos);
  positionupdate.setArg(1, particledata);
  positionupdate.setArg(2, settings.particle_count);
  positionupdate.setArg(3, settings.position_update_items_per_thread);
  positionupdate.setArg(4, settings.timestep);
  return requires_restart;
}

// Initialise OpenCL, attach OpenGL Buffers

constexpr size_t add8powers(const size_t exp) {
  size_t res = 0;
  for (size_t i = 0; i <= exp; i++) {
    res += 1uLL << (3uLL * i);
  }
  return res;
}

void NBody::doTesting() {
  std::ofstream File("test.txt");
  std::vector<Node> nodes(settings.allocatedNodes);
  std::vector<cl_float3> GPUpos(settings.particle_count);
  std::vector<ParticleData> GPUdata(settings.particle_count);
  cl_float3 min;
  cl_float3 max;
  cl_int itr;

  command_queue.enqueueReadBuffer(Nodes, CL_FALSE, 0,
                                  nodes.size() * sizeof(Node), nodes.data());
  command_queue.enqueueReadBuffer(particlepos, CL_FALSE, 0,
                                  settings.particle_count * sizeof(cl_float3),
                                  GPUpos.data());
  command_queue.enqueueReadBuffer(particledata, CL_FALSE, 0,
                                  GPUdata.size() * sizeof(ParticleData),
                                  GPUdata.data());

  command_queue.enqueueReadBuffer(globalMinBuffer, CL_FALSE, 0,
                                  sizeof(cl_float3), &min);
  command_queue.enqueueReadBuffer(globalMaxBuffer, CL_FALSE, 0,
                                  sizeof(cl_float3), &max);
  command_queue.enqueueReadBuffer(itrBuffer, CL_FALSE, 0, sizeof(cl_int), &itr);
  command_queue.finish();
  File << "min: " << min << " max: " << max << std::endl;
  //  std::cout << "Found min: " << min << " max: " << max << std::endl;
  //
  //  cl_float3 mi = {{FLT_MAX, FLT_MAX, FLT_MAX}};
  //  cl_float3 ma = {{FLT_MIN, FLT_MIN, FLT_MIN
  //}};
  //
  //  for (const cl_float3& p : pos) {
  //    mi = {{min(mi.x, p.x), min(mi.y, p.y), min(mi.z, p.z)}};
  //    ma = {{max(ma.x, p.x), max(ma.y, p.y), max(ma.z, p.z)}};
  //  }
  //
  //
  //
  //  std::cout << "Actual min: " << min << " max: " << max << std::endl;

  for (int i = 0; i < settings.particle_count; i++) {
    File << "Particle: " << i << " GPU " << GPUpos[i] << std::endl;

    File << "ParticleDATA: " << i << " GPU " << GPUdata[i] << std::endl;
  }

  for (int i = 0; i < add8powers(3); i++) {
    File << "index " << i << std::endl << nodes[i] << std::endl;
  }

  File << "SEP-----------------------------------------------------------------"
          "----------------"
       << std::endl;

  for (int i = add8powers(3); i < itr; i++) {
    File << "index " << i << std::endl << nodes[i] << std::endl;
  }
}

inline float getMSTime(cl::Event& ev) {
  cl_ulong start, end;
  ev.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
  ev.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
  return (end - start) / 1e+06;
}

void NBody::Calculate() {
  std::vector<cl::Event> ev1(1);
  std::vector<cl::Event> ev2(1);
  std::vector<cl::Event> ev3(1);
  std::vector<cl::Event> ev4(1);
  std::vector<cl::Event> ev51(1);
  std::vector<cl::Event> ev5(1);
  std::vector<cl::Event> ev6(1);
  std::vector<cl::Event> ev7(1);
  std::vector<cl::Event> evread(1);
  float truedt;
  try {
#ifdef DEBUG
    std::cout << "Starting calculation" << std::endl;
#endif
    command_queue.enqueueNDRangeKernel(
        boundingbox, cl::NullRange,
        cl::NDRange(global_work_size_from_work_groups(
            settings.particle_count, settings.boundingbox_work_group_size)),
        cl::NDRange(settings.boundingbox_work_group_size), nullptr, &ev1[0]);

    command_queue.enqueueNDRangeKernel(
        boundingboxstage2, cl::NullRange,
        cl::NDRange(global_work_size_from_work_groups(
            settings.particle_count, settings.boundingbox_work_group_size)),
        cl::NDRange(settings.boundingbox_work_group_size), &ev1, &ev2[0]);

    command_queue.enqueueNDRangeKernel(initOctree, cl::NullRange,
                                       cl::NDRange(1), cl::NDRange(1), &ev2,
                                       &ev3[0]);

    command_queue.enqueueNDRangeKernel(
        buildOctree, cl::NullRange,
        cl::NDRange((1uLL << (3uLL * settings.start_depth))),

        cl::NullRange, &ev3, &ev4[0]);
    int usedNodes;
    command_queue.enqueueReadBuffer(itrBuffer, CL_FALSE, 0, sizeof(cl_int),
                                    &usedNodes, &ev4, &evread[0]);

    // command_queue.enqueueNDRangeKernel(
    //     centerofMass, cl::NullRange,
    //     cl::NDRange(
    //         global_work_size(particle_count,
    //         center_of_mass_items_per_thread)),
    //     cl::NDRange(center_of_mass_items_per_thread));

    command_queue.enqueueNDRangeKernel(centerofMass, cl::NullRange,
                                       cl::NDRange(1), cl::NDRange(1), &ev4,
                                       &ev51[0]);
    command_queue.enqueueNDRangeKernel(
        DivideByMass, cl::NullRange,
        cl::NDRange(settings.divide_by_mass_threads), cl::NullRange, &ev51,
        &ev5[0]);

    command_queue.enqueueNDRangeKernel(
        barneshut, cl::NullRange,
        cl::NDRange(global_work_size_from_item_per_thread(
            settings.particle_count, settings.barneshut_items_per_thread)),
        cl::NullRange, &ev5, &ev6[0]);
#ifdef DEBUG
    cl::WaitForEvents(ev4);
    std::cout << "octree - done" << std::endl;
#endif
    cl::WaitForEvents(ev6);

#ifdef DEBUG
    std::cout << "barneshut - done" << std::endl;
#endif

    m_writing_mutex.lock();
    truedt = timer.Tick();
#if defined(_WIN32)
    float dt = min(truedt, 0.2f);
#elif defined(__linux__)
    float dt = std::min(truedt, 0.2f);
#endif

    positionupdate.setArg(4, dt);
    command_queue.enqueueNDRangeKernel(
        positionupdate, cl::NullRange,
        cl::NDRange(global_work_size_from_item_per_thread(
            settings.particle_count,
            settings.position_update_items_per_thread)),
        cl::NullRange, &ev6, &ev7[0]);

    cl::WaitForEvents(ev7);
    m_writing_mutex.unlock();
#ifdef DEBUG
    std::cout << "positionupdate - done" << std::endl;
#endif

    m_done_mutex.lock();
    m_newdata = true;
    m_done_mutex.unlock();

    cl::WaitForEvents(evread);
#
    simulation_results.usedNodes = usedNodes;
    simulation_results.allocatedNodes = settings.allocatedNodes;
    if (usedNodes > settings.allocatedNodes) {
      throw Error(CL_OUT_OF_RESOURCES, "Used more nodes than allocated");
    }
  } catch (cl::Error error) {
    throw error;
  }

  simulation_results.boundingboxstage1ms = getMSTime(ev1[0]);
  simulation_results.boundingboxstage2ms = getMSTime(ev2[0]);
  simulation_results.initOctreems = getMSTime(ev3[0]);
  simulation_results.buildOctreems = getMSTime(ev4[0]);
  simulation_results.centerofMassms = getMSTime(ev5[0]);
  simulation_results.dividecenterofmassms = getMSTime(ev51[0]);
  simulation_results.barneshutms = getMSTime(ev6[0]);
  simulation_results.positionupdatems = getMSTime(ev7[0]);
  simulation_results.deltaTimesecond = truedt;
}

void NBody::Clean() {}

void NBody::UpdateCommunication(Communication& comm) {
  comm.SetSimulationData(simulation_results);
}

void NBody::Start(

) {
  ParticleSetDescription set = settings.layout(settings.particle_count);

  std::vector<cl_float3> pos = set.first;
  pos.resize(settings.particle_count);
  std::vector<ParticleData> data = set.second;
  data.resize(settings.particle_count);

  try {
    // These have to be done here since we will transfer these over to openGL

    m_writing_mutex.lock();
    command_queue.enqueueWriteBuffer(
        particlepos, CL_FALSE, 0, pos.size() * sizeof(cl_float4), pos.data());

    command_queue.enqueueWriteBuffer(particledata, CL_FALSE, 0,
                                     data.size() * sizeof(ParticleData),
                                     data.data());

    command_queue.enqueueNDRangeKernel(createOctree, cl::NullRange,
                                       cl::NDRange(1), cl::NDRange(1));
    command_queue.finish();
    m_writing_mutex.unlock();
    std::lock_guard lock(m_done_mutex);
    m_newdata = true;
    // This can wait :)
  } catch (cl::Error error) {
    throw error;
  }
}

bool NBody::TryAndWriteData() {
  bool update = false;
  {
    std::lock_guard<std::mutex> done_lock(m_done_mutex);
    if (m_newdata && m_writing_mutex.try_lock()) {
      m_newdata = false;
      update = true;
    }
    // Let go of the lock as early as possible
  }
  // We have new data and successfully locked m_writing_mutex
  if (update) {
#ifdef DEBUG
    std::cout << "UPDATING DATA" << std::endl;
#endif
    try {
      std::vector<cl::Event> ev1(1);
      std::vector<cl::Event> ev2(1);
      std::vector<cl::Event> ev3(1);
      copy_command_queue.enqueueAcquireGLObjects(&GLbuffers, nullptr, &ev1[0]);
      copy_command_queue.enqueueCopyBuffer(
          particlepos, openGLparticlepos, 0, 0,
          settings.particle_count * sizeof(cl_float4), &ev1, &ev2[0]);
      copy_command_queue.enqueueReleaseGLObjects(&GLbuffers, &ev2, &ev3[0]);
      cl::WaitForEvents(ev3);
    } catch (cl::Error error) {
      throw error;
    }
    m_writing_mutex.unlock();
  }
  return update;
}
