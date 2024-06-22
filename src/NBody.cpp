#include "NBody.h"

#define __NO_STD_VECTOR
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <fstream>
#include <iostream>
#include <oclutils.hpp>
#include <random>
#include <string>
#include <utility>
using namespace cl;
void NBody::TryAndWriteData() {
  bool update = false;
  {
    std::lock_guard done_lock(m_done_mutex);
    if (m_newdata && m_writing_mutex.try_lock()) {
      m_newdata = false;
      update = true;
    }
    // Let go of the lock as early as possible
  }
  // We have new data and successfully locked m_writing_mutex
  if (update) {
    // TODO: Finish
    m_writing_mutex.unlock();
  }
}

ParticleSetDescription UniformLayout(const size_t size) {
  std::vector<glm::vec3> particles;
  std::vector<ParticleData> particle_data(size);
  particles.reserve(size);

  std::default_random_engine generator;
  std::normal_distribution<float> distribution(0.0, 1.0);

  for (size_t i = 0; i < size; ++i) {
    float x = distribution(generator);
    float y = distribution(generator);
    float z = distribution(generator);
    particles.emplace_back(glm::vec3(x, y, z));
  }

  return std::make_pair(particles, particle_data);
}

ParticleSetDescription EvenLayout(const size_t count) {
  std::vector<glm::vec3> particles;
  std::vector<ParticleData> particle_data(count);
  const int num = std::ceil((std::cbrt(static_cast<float>(count)) - 1.) / 2.);
  const size_t res = static_cast<size_t>(std::pow(num * 2 + 1, 3));

  particles.reserve(res);
  glm::vec3 offset(0.f, 0.f, 0.f);
  glm::vec3 size(1.f, 1.f, 1.f);
  for (int i = -num; i <= num; i++) {
    for (int j = -num; j <= num; j++) {
      for (int k = -num; k <= num; k++) {
        glm::vec3 pos = offset + size * glm::vec3(static_cast<float>(i),
                                                  static_cast<float>(j),
                                                  static_cast<float>(k));
        particles.emplace_back(pos);
      }
    }
  }
  return std::make_pair(particles, particle_data);
}

NBody::NBody() {}
NBody::~NBody() {}

bool NBody::Init(GLuint VBOIndex) {
  try {
    ///////////////////////////
    // Initialize OpenCL API //
    ///////////////////////////

    vector<cl::Platform> platforms;
    Platform::get(&platforms);

    // Try to get the sharing platform!
    bool create_context_success = false;
    for (auto platform : platforms) {
      // Next, create an OpenCL context on the platform.  Attempt to
      // create a GPU-based context.
      cl_context_properties contextProperties[] = {
#ifdef _WIN32
          CL_CONTEXT_PLATFORM,
          (cl_context_properties)(platform)(),
          CL_GL_CONTEXT_KHR,
          (cl_context_properties)wglGetCurrentContext(),
          CL_WGL_HDC_KHR,
          (cl_context_properties)wglGetCurrentDC(),
#elif defined(__GNUC__)
          CL_CONTEXT_PLATFORM,
          (cl_context_properties)(platform)(),
          CL_GL_CONTEXT_KHR,
          (cl_context_properties)glXGetCurrentContext(),
          CL_GLX_DISPLAY_KHR,
          (cl_context_properties)glXGetCurrentDisplay(),
#elif defined(__APPLE__)
      // todo
#endif
          0};

      // Create Context
      try {
        context = cl::Context(CL_DEVICE_TYPE_GPU, contextProperties);
        create_context_success = true;
        break;
      } catch (Error error) {
      }
    }

    if (!create_context_success)
      throw cl::Error(CL_INVALID_CONTEXT,
                      "Failed to create CL/GL shared context");

    // Create Command Queue
    cl::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    command_queue = cl::CommandQueue(context, devices[0]);

    /////////////////////////////////
    // Load, then build the kernel //
    /////////////////////////////////

    // Read source file
    std::ifstream sourceFile("GLinterop.cl");
    std::string sourceCode(std::istreambuf_iterator<char>(sourceFile),
                           (std::istreambuf_iterator<char>()));
    cl::Program::Sources source(
        1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));

    // Make program of the source code in the context
    program = cl::Program(context, source);
    try {
      program.build(devices);
    } catch (cl::Error error) {
      std::cout << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0])
                << std::endl;
      throw error;
    }

    // Make kernel
    kernel_tex = cl::Kernel(program, "texture_kernel");

    // Create Mem Objs
    cl_tex_mem =
        cl::Image2DGL(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, texture);

    // Query textures
    performTexQuery();  // Just to check..
  } catch (cl::Error error) {
    std::cout << error.what() << "(" << oclErrorString(error.err()) << ")"
              << std::endl;
    return false;
  }
  return true;
}
void NBody::Start(
    const size_t particle_count,
    std::function<ParticleSetDescription(const size_t)> generating_func,
    std::optional<const size_t> allocate_particle_count) {
  ParticleSetDescription set = generating_func(particle_count);
  std::vector<glm::vec3> pos = set.first;
  std::vector<ParticleData> data = set.second;
  size_t count = allocate_particle_count.value_or(0) + particle_count;
  pos.resize(count);
  data.resize(count);
}

void NBody::Clean() {}

void NBody::Calculate() {}