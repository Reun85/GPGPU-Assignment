#include "NBody.h"
// #define DEBUG

#include <CL/cl_gl.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <windows.h>

#include <CL/opencl.hpp>
#include <iostream>
#include <vector>

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
     << " "
     << n.center_of_mass.z << std::endl;
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
      // copy_command_queue.enqueueAcquireGLObjects(&GLbuffers, nullptr,
      // &ev1[0]);
      //  copy_command_queue.enqueueCopyBuffer(particlepos, openGLparticlepos,
      //  0, 0,
      //                                      particle_count *
      //                                      sizeof(glm::vec3), &ev1, &ev2[0]);
      //  copy_command_queue.enqueueReleaseGLObjects(&GLbuffers, &ev2, &ev3[0]);
      copy_command_queue.enqueueAcquireGLObjects(&GLbuffers, nullptr, &ev1[0]);
      copy_command_queue.enqueueCopyBuffer(particlepos, openGLparticlepos, 0, 0,
                                           particle_count * sizeof(cl_float4),
                                           &ev1, &ev2[0]);
      copy_command_queue.enqueueReleaseGLObjects(&GLbuffers, &ev2, &ev3[0]);
      // cl::WaitForEvents(ev3);
    } catch (cl::Error error) {
      std::cout << error.what() << "(" << oclErrorString(error.err()) << ")"
                << std::endl;
      std::exit(1);
    }
    m_writing_mutex.unlock();
  }
  return update;
}

ParticleSetDescription UniformLayout(const size_t size,
                                     const float default_mass) {
  std::vector<cl_float3> particles;
  cl_float3 empt;
  empt.x = 0;
  empt.y = 0;
  empt.z = 0;
  ParticleData def_data{empt, empt};
  std::vector<ParticleData> particle_data(size, def_data);
  particles.reserve(size);

  std::default_random_engine generator;
  std::normal_distribution<float> distribution(0.0, 1.0);

  for (size_t i = 0; i < size; ++i) {
    float x = distribution(generator);
    float y = distribution(generator);
    float z = distribution(generator);
    cl_float3 t;
    t.x = x;
    t.y = y;
    t.z = z;
    t.w = default_mass;

    particles.push_back(t);
  }

  return std::make_pair(particles, particle_data);
}

ParticleSetDescription Galaxy(const size_t size,
                                  const float default_mass) {
    const glm::vec3 g_center = glm::vec3(0, 0, 0);
    const glm::vec3 g_velocity = glm::vec3(0, 0, 0);


  std::vector<cl_float3> particles;
    std::vector<ParticleData> particle_data;
  particles.reserve(size);
  particle_data.reserve(size);

  std::default_random_engine generator;
  std::normal_distribution<float> distribution(0.0, 1.0);

  float PI = 3.14159265359;
  for (size_t i = 0; i < size; ++i) {
    float rng1 = distribution(generator) * 2.f * PI;
    float rng2 = distribution(generator);
    float rng3 = distribution(generator);
    float rng4 = distribution(generator);
    rng4 = pow((rng4 + 1.0f) / 2.0f, 1.0f)*7500.0f;  // 0.5 ~ 1.0
    glm::vec4 _pos = glm::vec4(cos(rng1) * rng2 * 1.0f, rng3 / 20.0f,
                                  sin(rng1) * rng2 * 1.0f, rng4);
    _pos += glm::vec4(g_center, 0.f);  // galaxy 1 center

    float r = 1 - distribution(generator) / 10.0f;
    glm::vec3 tang_vel =
        glm::vec3(glm::normalize(glm::cross(glm::vec3(0, 1, 0),
                                            glm::vec3(_pos) - g_center))
                  );
    float dis = glm::distance(glm::vec3(_pos), g_center);
    glm::vec3 rnd_vel = tang_vel * (dis)*25.f;
    rnd_vel /= 100;

    rnd_vel += g_velocity;  // galaxy 1 center

 particles.push_back({{ _pos.x, _pos.y, _pos.z, _pos.w }});
	particle_data.push_back({{rnd_vel.x, rnd_vel.y, rnd_vel.z},
        							  {{0, 0, 0}}});


        
  }
  return std::make_pair(particles, particle_data);
}

NBody::NBody() {}
NBody::~NBody() {}

bool NBody::Init(GLuint VBOIndex, const size_t particle_num,
                 const size_t extra_allocate_particle_count) {
  particle_count = particle_num;
  allocated_particle_count = extra_allocate_particle_count + particle_count;

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
    std::ifstream sourceFile("openclkernels.c");
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
    Nodes =
        cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Node) * allocatedNodes);
    particledata = cl::Buffer(context, CL_MEM_READ_WRITE,
                              sizeof(ParticleData) * allocated_particle_count);

    particlepos = cl::Buffer(context, CL_MEM_READ_WRITE,
                             sizeof(cl_float4) * allocated_particle_count);
    openGLparticlepos = cl::BufferGL(context, CL_MEM_WRITE_ONLY, VBOIndex);

    // Intermediate buffers
    minValuesBuffer =
        cl::Buffer(context, CL_MEM_READ_WRITE,
                   sizeof(cl_float3) * boundingbox_work_group_size, NULL);
    maxValuesBuffer =
        cl::Buffer(context, CL_MEM_READ_WRITE,
                   sizeof(cl_float3) * boundingbox_work_group_size, NULL);
    globalMinBuffer =
        cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(cl_float3), NULL);
    globalMaxBuffer =
        cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(cl_float3), NULL);
    itrBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(cl_int), NULL);

    GLbuffers.push_back(openGLparticlepos);

    // Make kernel
    boundingbox = cl::Kernel(program, "BoundingBoxStage1");

    boundingbox.setArg(0, particlepos);
    boundingbox.setArg(1, minValuesBuffer);
    boundingbox.setArg(2, maxValuesBuffer);
    boundingbox.setArg(3, static_cast<const int>(particle_count));
    boundingbox.setArg(4, boundingbox_work_group_size);

    boundingboxstage2 = cl::Kernel(program, "BoundingBoxStage2");
    boundingboxstage2.setArg(0, minValuesBuffer);
    boundingboxstage2.setArg(1, maxValuesBuffer);
    boundingboxstage2.setArg(2, globalMinBuffer);
    boundingboxstage2.setArg(3, globalMaxBuffer);
    boundingboxstage2.setArg(4, boundingbox_work_group_size);

    createOctree = cl::Kernel(program, "CreateOctree");
    createOctree.setArg(0, Nodes);
    createOctree.setArg(1, static_cast<const int>(START_DEPTH));

    initOctree = cl::Kernel(program, "InitOctree");
    initOctree.setArg(0, Nodes);
    initOctree.setArg(1, static_cast<const int>(START_DEPTH));
    initOctree.setArg(2, globalMinBuffer);
    initOctree.setArg(3, globalMaxBuffer);
    initOctree.setArg(4, itrBuffer);
    initOctree.setArg(5, static_cast<const int>(allocatedNodes));

    buildOctree = cl::Kernel(program, "BuildOctree");
    buildOctree.setArg(0, particlepos);
    buildOctree.setArg(1, Nodes);
    buildOctree.setArg(2, globalMinBuffer);
    buildOctree.setArg(3, globalMaxBuffer);
    buildOctree.setArg(4, static_cast<const int>(particle_count));
    buildOctree.setArg(5, static_cast<const int>(START_DEPTH));
    buildOctree.setArg(6, itrBuffer);
    buildOctree.setArg(7, octree_items_per_thread);

    DivideByMass = cl::Kernel(program, "DivideCentersByMass");
    DivideByMass.setArg(0, Nodes);
    DivideByMass.setArg(1, itrBuffer);

    centerofMass = cl::Kernel(program, "CalculateCenterOfMass");
    centerofMass.setArg(0, Nodes);
    centerofMass.setArg(1, static_cast<const int>(START_DEPTH));
    centerofMass.setArg(2, itrBuffer);

    barneshut = cl::Kernel(program, "BarnesHut");
    barneshut.setArg(0, particlepos);
    barneshut.setArg(1, particledata);
    barneshut.setArg(2, Nodes);
    barneshut.setArg(3, static_cast<const int>(particle_count));
    barneshut.setArg(4, distanceThreshold);
    barneshut.setArg(5, eps);
    barneshut.setArg(6, gravitational_constant);
    barneshut.setArg(7, barneshut_items_per_thread);

    positionupdate = cl::Kernel(program, "AddForces");
    positionupdate.setArg(0, particlepos);
    positionupdate.setArg(1, particledata);
    positionupdate.setArg(2, static_cast<const int>(particle_count));
    positionupdate.setArg(3, position_update_items_per_thread);
    positionupdate.setArg(4, timestep);
  } catch (cl::Error error) {
    std::cout << error.what() << "(" << oclErrorString(error.err()) << ")"
              << std::endl;
    return false;
  }

  return true;
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
  std::vector<Node> nodes(allocatedNodes);
  std::vector<cl_float3>& pos = test_positions;
  std::vector<cl_float3> GPUpos(pos.size());
  std::vector<ParticleData>& data = test_data;
  std::vector<ParticleData> GPUdata(data.size());
  cl_float3 min;
  cl_float3 max;
  cl_int temp;
  cl_int itr;

  command_queue.enqueueReadBuffer(Nodes, CL_FALSE, 0,
                                  nodes.size() * sizeof(Node), nodes.data());
  command_queue.enqueueReadBuffer(
      particlepos, CL_FALSE, 0, pos.size() * sizeof(cl_float3), GPUpos.data());
  command_queue.enqueueReadBuffer(particledata, CL_FALSE, 0,
                                  GPUdata.size() * sizeof(ParticleData),
                                  GPUdata.data());

  command_queue.enqueueReadBuffer(globalMinBuffer, CL_FALSE, 0,
                                  sizeof(cl_float3), &min);
  command_queue.enqueueReadBuffer(globalMaxBuffer, CL_FALSE, 0,
                                  sizeof(cl_float3), &max);
  command_queue.enqueueReadBuffer(itrBuffer, CL_FALSE, 0, sizeof(cl_int), &itr);
  command_queue.finish();
  File << pos.size() << std::endl;
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

  for (int i = 0; i < pos.size(); i++) {
    if (pos[i].x != GPUpos[i].x || pos[i].y != GPUpos[i].y ||
        pos[i].z != GPUpos[i].z || pos[i].w != GPUpos[i].w)
      File << "Particle: " << i << " real " << pos[i] << " GPU " << GPUpos[i]
           << std::endl;

    if (data[i] != GPUdata[i])
      File << "ParticleDATA: " << i << " real " << data[i] << " GPU "
           << GPUdata[i] << std::endl;
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

  // std::exit(1);
}

void printTime(cl::Event& ev, std::string name) {
  cl_ulong start, end;
  ev.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
  ev.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
  std::cout << name << " took " << (end - start) / 1e+06 << "ms" << std::endl;
}

void NBody::Calculate(NBodyTimer& timer) {
  std::vector<cl::Event> ev1(1);
  std::vector<cl::Event> ev2(1);
  std::vector<cl::Event> ev3(1);
  std::vector<cl::Event> ev4(1);
  std::vector<cl::Event> ev42(1);
  std::vector<cl::Event> ev5(1);
  std::vector<cl::Event> ev6(1);
  std::vector<cl::Event> ev7(1);
  std::vector<cl::Event> evread(1);
  try {
#ifdef DEBUG
    std::cout << "Starting calculation" << std::endl;
#endif
    command_queue.enqueueNDRangeKernel(
        boundingbox, cl::NullRange,
        cl::NDRange(global_work_size_from_work_groups(
            particle_count, boundingbox_work_group_size)),
        cl::NDRange(boundingbox_work_group_size), nullptr, &ev1[0]);

    command_queue.enqueueNDRangeKernel(
        boundingboxstage2, cl::NullRange,
        cl::NDRange(global_work_size_from_work_groups(
            particle_count, boundingbox_work_group_size)),
        cl::NDRange(boundingbox_work_group_size), &ev1, &ev2[0]);

    command_queue.enqueueNDRangeKernel(initOctree, cl::NullRange,
                                       cl::NDRange(1), cl::NDRange(1), &ev2,
                                       &ev3[0]);

    command_queue.enqueueNDRangeKernel(
        buildOctree, cl::NullRange,
        cl::NDRange(global_work_size_from_item_per_thread(
            (1uLL << (3uLL * START_DEPTH)), octree_items_per_thread)),
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
                                       &ev42[0]);
    command_queue.enqueueNDRangeKernel(DivideByMass, cl::NullRange,
                                       cl::NDRange(divide_by_mass_threads), cl::NullRange, &ev42,
                                       &ev5[0]);
    command_queue.enqueueNDRangeKernel(
        barneshut, cl::NullRange,
        cl::NDRange(global_work_size_from_item_per_thread(
            particle_count, barneshut_items_per_thread)),
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
    float dt = timer.Tick();
    std::cout << "UPS: " << 1.f / dt << std::endl;
    dt = min(dt, 0.2f);

    positionupdate.setArg(4, dt);
    command_queue.enqueueNDRangeKernel(
        positionupdate, cl::NullRange,
        cl::NDRange(global_work_size_from_item_per_thread(
            particle_count, position_update_items_per_thread)),
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
    std::cout << "Used nodes: " << usedNodes << " Allocated: "<<allocatedNodes<< std::endl;
    if (usedNodes > allocatedNodes) {
      std::cout << "Used more nodes than allocated, exiting" << std::endl;
      std::exit(1);
    }
  } catch (cl::Error error) {
    std::cout << error.what() << "(" << oclErrorString(error.err()) << ")"
              << std::endl;

    std::exit(1);
  }

  printTime(ev1[0], "boundingbox");
  printTime(ev2[0], "boundingboxstage2");
  printTime(ev3[0], "initOctree");
  printTime(ev4[0], "buildOctree");
  printTime(ev5[0], "centerofMass");
  printTime(ev6[0], "barneshut");
  printTime(ev7[0], "positionupdate");
}

void NBody::Clean() {}

void NBody::Start(

    std::function<ParticleSetDescription(const size_t, const float)>
        generating_func) {
  ParticleSetDescription set = generating_func(particle_count, default_mass);
  std::vector<cl_float3> pos = set.first;
  std::vector<ParticleData> data = set.second;
  pos.resize(allocated_particle_count);
  data.resize(allocated_particle_count);
  test_positions = set.first;
  test_data = set.second;

  try {
    // These have to be done here since we will transfer these over to openGL

    command_queue.enqueueWriteBuffer(
        particlepos, CL_FALSE, 0, pos.size() * sizeof(cl_float3), pos.data());

    command_queue.enqueueWriteBuffer(particledata, CL_FALSE, 0,
                                     data.size() * sizeof(ParticleData),
                                     data.data());

    command_queue.enqueueNDRangeKernel(createOctree, cl::NullRange,
                                       cl::NDRange(1), cl::NDRange(1));
    command_queue.finish();
    // This can wait :)
  } catch (cl::Error error) {
    std::cout << error.what() << "(" << oclErrorString(error.err()) << ")"
              << std::endl;
    std::exit(1);
  }
}