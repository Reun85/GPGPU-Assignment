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
#include "Camera.h"
#include "GLUtils.hpp"

struct SUpdateInfo {
  float ElapsedTimeInSec = 0.0f; // Program indulása óta eltelt idő
  float DeltaTimeInSec = 0.0f;   // Előző Update óta eltelt idő
};

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

protected:
  void SetupDebugCallback();

  //
  // Adat változók
  //

  float m_ElapsedTimeInSec = 0.0f;

  // Kamera
  Camera m_camera;

  //
  // OpenGL-es dolgok
  //

  // uniform location lekérdezése
  GLint ul(const char *uniformName) noexcept;

  // shaderekhez szükséges változók
  GLuint m_programID = 0;      // shaderek programja
  GLuint m_programPointID = 0; // skybox programja
  GLuint m_programAxesID = 0;  // shaderek programja

  // Fényforrás- ...
  glm::vec4 m_lightPos = glm::vec4(0, 0, 0, 0.0f);

  glm::vec3 m_La = glm::vec3(0.125f);
  glm::vec3 m_Ld = glm::vec3(1.0, 1.0, 1.0);
  glm::vec3 m_Ls = glm::vec3(1.0, 1.0, 1.0);

  float m_lightConstantAttenuation = 1.0;
  float m_lightLinearAttenuation = 10.0;
  float m_lightQuadraticAttenuation = 10.0;

  // ... és anyagjellemzők
  glm::vec3 m_Ka = glm::vec3(1.0);
  glm::vec3 m_Kd = glm::vec3(1.0);
  glm::vec3 m_Ks = glm::vec3(1.0);

  float m_Shininess = 1.0;

  // Shaderek inicializálása, és törtlése
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
