#pragma once

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// Utils
#include <vector>

#include "Camera.h"
#include "SUpdateInfo.h"

class CMyApp {
 public:
  CMyApp();
  ~CMyApp();

  bool Init();
  void Clean();

  void Update(const SUpdateInfo &);
  void Render();
  void RenderGUI();

  void KeyboardDown(const SDL_KeyboardEvent &);
  void KeyboardUp(const SDL_KeyboardEvent &);
  void MouseMove(const SDL_MouseMotionEvent &);
  void MouseDown(const SDL_MouseButtonEvent &);
  void MouseUp(const SDL_MouseButtonEvent &);
  void MouseWheel(const SDL_MouseWheelEvent &);
  void Resize(int, int);

  GLuint GetVBOAddress() const { return VBO; }
  int GetParticleCount() const { return particle_count; }

  void SetParticleCount(int _count);
  void UpdatedParticles();

 protected:
  void SetupDebugCallback();

  // ── Data ────────────────────────────────────────────────────────────
  float m_ElapsedTimeInSec = 0.0f;
  int particle_count = 0;
  bool needstoupdate = false;

  // ── Camera ──────────────────────────────────────────────────────────
  Camera m_camera;

  // ╭─────────────────────────────────────────────────────────╮
  // │                      OpenGL stuff                       │
  // ╰─────────────────────────────────────────────────────────╯

  /// Uniform location
  GLint ul(const char *uniformName) noexcept;

  GLuint VAO = 0;
  GLuint VBO = 0;

  GLuint m_programID = 0;
  GLuint m_programPointID = 0;
  GLuint m_programAxesID = 0;

  // glm::vec4 m_lightPos = glm::vec4(0, 0, 0, 0.0f);
  //
  // glm::vec3 m_La = glm::vec3(0.125f);
  // glm::vec3 m_Ld = glm::vec3(1.0, 1.0, 1.0);
  // glm::vec3 m_Ls = glm::vec3(1.0, 1.0, 1.0);
  //
  // float m_lightConstantAttenuation = 1.0;
  // float m_lightLinearAttenuation = 10.0;
  // float m_lightQuadraticAttenuation = 10.0;

  // glm::vec3 m_Ka = glm::vec3(1.0);
  // glm::vec3 m_Kd = glm::vec3(1.0);
  // glm::vec3 m_Ks = glm::vec3(1.0);
  //
  // float m_Shininess = 1.0;

  // Shaderek inicializálása, és törlése
  void InitShaders();
  void CleanShaders();
  void InitSkyboxShaders();
  void CleanSkyboxShaders();

  // Geometriával kapcsolatos változók

  // Geometria inicializálása, és törtlése
  void InitGeometry();
  void CleanGeometry();
  void InitSkyboxGeometry();
  void CleanSkyboxGeometry();

  // Textúrázás, és változói

  void InitTextures();
  void CleanTextures();
  void InitSkyboxTextures();
  void CleanSkyboxTextures();
};
