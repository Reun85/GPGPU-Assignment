#include "MyApp.h"

#include <imgui.h>

#include <cassert>
#include <cmath>
#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>

#include "ObjParser.h"
#include "ParametricSurfaceMesh.hpp"
#include "SDL_GLDebugMessageCallback.h"

CMyApp::CMyApp(int PARTICLE_SIZE) { count = PARTICLE_SIZE; }

CMyApp::~CMyApp() {}

void CMyApp::SetupDebugCallback() {
  // engedélyezzük és állítsuk be a debug callback függvényt ha debug
  // context-ben vagyunk
  GLint context_flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
  if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
                          GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    glDebugMessageCallback(SDL_GLDebugMessageCallback, nullptr);
  }
}

void CMyApp::InitShaders() {
  m_programID = glCreateProgram();
  AssembleProgram(m_programID, "shaders/Vert_PosNormTex.vert",
                  "shaders/Frag_Lighting.frag");
  m_programAxesID = glCreateProgram();
  AssembleProgram(m_programAxesID, "shaders/Vert_axes.vert",
                  "shaders/Frag_axes.frag");
  m_programPointID = glCreateProgram();
  AssembleProgram(m_programPointID, "shaders/Vert_Point.vert",
                  "shaders/Frag_PosCol.frag");

  InitSkyboxShaders();
}

void CMyApp::InitSkyboxShaders() {
  // m_programSkyboxID = glCreateProgram();
  // AssembleProgram(m_programSkyboxID, "Vert_skybox.glsl", "Frag_skybox.glsl");
}

void CMyApp::CleanShaders() {
  glDeleteProgram(m_programID);
  glDeleteProgram(m_programAxesID);
  glDeleteProgram(m_programPointID);

  CleanSkyboxShaders();
}

void CMyApp::CleanSkyboxShaders() {
  // glDeleteProgram(m_programSkyboxID);
}

void CMyApp::CleanGeometry() { CleanSkyboxGeometry();

glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);

}

void CMyApp::InitSkyboxGeometry() {
  // skybox geo
  MeshObject<glm::vec3> skyboxCPU = {std::vector<glm::vec3>{
                                         // hátsó lap
                                         glm::vec3(-1, -1, -1),
                                         glm::vec3(1, -1, -1),
                                         glm::vec3(1, 1, -1),
                                         glm::vec3(-1, 1, -1),
                                         // elülső lap
                                         glm::vec3(-1, -1, 1),
                                         glm::vec3(1, -1, 1),
                                         glm::vec3(1, 1, 1),
                                         glm::vec3(-1, 1, 1),
                                     },

                                     std::vector<GLuint>{
                                         // hátsó lap
                                         0,
                                         1,
                                         2,
                                         2,
                                         3,
                                         0,
                                         // elülső lap
                                         4,
                                         6,
                                         5,
                                         6,
                                         4,
                                         7,
                                         // bal
                                         0,
                                         3,
                                         4,
                                         4,
                                         3,
                                         7,
                                         // jobb
                                         1,
                                         5,
                                         2,
                                         5,
                                         6,
                                         2,
                                         // alsó
                                         1,
                                         0,
                                         4,
                                         1,
                                         4,
                                         5,
                                         // felső
                                         3,
                                         2,
                                         6,
                                         3,
                                         6,
                                         7,
                                     }};

   //m_SkyboxGPU = CreateGLObjectFromMesh(
   //    skyboxCPU, {{0, offsetof(glm::vec3, x), 3, GL_FLOAT}});
}

void CMyApp::CleanSkyboxGeometry() {
  // CleanOGLObject(m_SkyboxGPU);
}

void CMyApp::InitTextures() {
  // diffuse texture

  // glGenTextures(1, &m_SuzanneTextureID);
  // TextureFromFile(m_SuzanneTextureID, "Assets/wood.jpg");
  // SetupTextureSampling(GL_TEXTURE_2D, m_SuzanneTextureID);
  InitSkyboxTextures();
}

void CMyApp::CleanTextures() {
  // glDeleteTextures(1, &m_SuzanneTextureID);

  CleanSkyboxTextures();
}

void CMyApp::InitSkyboxTextures() {
  // skybox texture

  // glGenTextures(1, &m_skyboxTextureID);
  // TextureFromFile(m_skyboxTextureID, "Assets/lab_xpos.png",
  // GL_TEXTURE_CUBE_MAP,
  //                 GL_TEXTURE_CUBE_MAP_POSITIVE_X);
  // TextureFromFile(m_skyboxTextureID, "Assets/lab_xneg.png",
  // GL_TEXTURE_CUBE_MAP,
  //                 GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
  // TextureFromFile(m_skyboxTextureID, "Assets/lab_ypos.png",
  // GL_TEXTURE_CUBE_MAP,
  //                 GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
  // TextureFromFile(m_skyboxTextureID, "Assets/lab_yneg.png",
  // GL_TEXTURE_CUBE_MAP,
  //                 GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
  // TextureFromFile(m_skyboxTextureID, "Assets/lab_zpos.png",
  // GL_TEXTURE_CUBE_MAP,
  //                 GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
  // TextureFromFile(m_skyboxTextureID, "Assets/lab_zneg.png",
  // GL_TEXTURE_CUBE_MAP,
  //                 GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
  // SetupTextureSampling(GL_TEXTURE_CUBE_MAP, m_skyboxTextureID, false);
  //
  // glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void CMyApp::CleanSkyboxTextures() {
  // glDeleteTextures(1, &m_skyboxTextureID);
}

bool CMyApp::Init() {
  SetupDebugCallback();

  glClearColor(0, 0, 0, 1.0f);

  // Nem minden driver támogatja a vonalak és pontok vastagabb
  // megjelenítését, ezért lekérdezzük, hogy támogatott-e a
  // GL_LINE_WIDTH_RANGE és GL_POINT_SIZE_RANGE tokenek.
  {
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glPointSize.xhtml
    GLfloat pointSizeRange[2] = {0.0f, 0.0f};
    glGetFloatv(GL_POINT_SIZE_RANGE,
                pointSizeRange);  // lekérdezzük a támogatott pontméretek
    // tartományát
    glPointSize(std::min(16.0f, pointSizeRange[1]));  // nagyobb pontok
  }

  {
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glLineWidth.xhtml
    GLfloat lineWidthRange[2] = {0.0f, 0.0f};
    glGetFloatv(GL_LINE_WIDTH_RANGE,
                lineWidthRange);  // lekérdezzük a támogatott
    // vonalvastagság tartományát
    glLineWidth(std::min(4.0f, lineWidthRange[1]));  // vastagabb vonalak
  }

  InitShaders();
  InitGeometry();
  InitTextures();

  //
  // egyéb inicializálás
  //

  glEnable(GL_CULL_FACE);  // kapcsoljuk be a hátrafelé néző lapok eldobását
  glCullFace(GL_BACK);     // GL_BACK: a kamerától "elfelé" néző lapok,
  // GL_FRONT: a kamera felé néző lapok

  glEnable(GL_DEPTH_TEST);  // mélységi teszt bekapcsolása (takarás)

  // kamera
  m_camera.SetView(
      glm::vec3(0.0, 7.0, 7.0),   // honnan nézzük a színteret	   - eye
      glm::vec3(0.0, 0.0, 0.0),   // a színtér melyik pontját nézzük - at
      glm::vec3(0.0, 1.0, 0.0));  // felfelé mutató irány a világban - up

  return true;
}

void CMyApp::Clean() {
  CleanShaders();
  CleanGeometry();
  CleanTextures();
}

void CMyApp::Update(const SUpdateInfo &updateInfo) {
  m_ElapsedTimeInSec = updateInfo.ElapsedTimeInSec;

  m_camera.Update(updateInfo.DeltaTimeInSec);
  // A table flip megnyomása után az objektumokat frissítjük.
}

void CMyApp::InitGeometry() {
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // Create and bind a Vertex Buffer Object (VBO)
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  std::vector<vec3> vertices(count);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
                  vertices.data(),GL_STATIC_DRAW);

  // Specify the layout of the vertex data
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3),
                        reinterpret_cast<const void *>(
                            0));  // a 0. indexű attribútum hol kezdődik a
  glEnableVertexAttribArray(0);

  // Unbind the VAO
  glBindVertexArray(0);

  // Skybox
  InitSkyboxGeometry();
}
void CMyApp::SetParticles(const std::vector<vec3> &vertices) {
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3),
                  vertices.data());
  glBindVertexArray(0);
  count = vertices.size();
}

void CMyApp::Render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //
  // Axes
  //
  {
    glBindVertexArray(0);
    glUseProgram(m_programAxesID);

    glUniformMatrix4fv(ul("world"), 1, GL_FALSE,
                       glm::value_ptr(glm::identity<glm::mat4>()));

    glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE,
                       glm::value_ptr(m_camera.GetViewProj()));

    glDrawArrays(GL_LINES, 0, 6);

    glUseProgram(0);
  }

  glUseProgram(m_programPointID);

  // Bind the VAO and draw points
  glBindVertexArray(VAO);
  glPointSize(1.5);
    glUniformMatrix4fv(ul("world"), 1, GL_FALSE,
                       glm::value_ptr(glm::identity<glm::mat4>()));

    glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE,
                       glm::value_ptr(m_camera.GetViewProj()));
  glDrawArrays(GL_POINTS, 0, count);
  glBindVertexArray(0);

  //
  // skybox
  //
  // {
  //   // - VAO
  //   glBindVertexArray(m_SkyboxGPU.vaoID);
  //
  //   // - Textura
  //   glActiveTexture(GL_TEXTURE0);
  //   glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTextureID);
  //
  //   // - Program
  //   glUseProgram(m_programSkyboxID);
  //
  //   // - uniform parameterek
  //   glUniformMatrix4fv(ul("world"), 1, GL_FALSE,
  //                      glm::value_ptr(glm::translate(m_camera.GetEye())));
  //   glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE,
  //                      glm::value_ptr(m_camera.GetViewProj()));
  //
  //   // - textúraegységek beállítása
  //   glUniform1i(ul("skyboxTexture"), 0);
  //
  //   // mentsük el az előző Z-test eredményt, azaz azt a relációt, ami
  //   // alapján update-eljük a pixelt.
  //   GLint prevDepthFnc;
  //   glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFnc);
  //
  //   // most kisebb-egyenlőt használjunk, mert mindent kitolunk a távoli
  //   // vágósíkokra
  //   glDepthFunc(GL_LEQUAL);
  //
  //   // - Rajzolas
  //   glDrawElements(GL_TRIANGLES, m_SkyboxGPU.count, GL_UNSIGNED_INT,
  //   nullptr);
  //
  //   glDepthFunc(prevDepthFnc);
  // }
  // shader kikapcsolasa
  glUseProgram(0);

  // - Textúrák kikapcsolása, minden egységre külön
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  // VAO kikapcsolása
  glBindVertexArray(0);
}

void CMyApp::RenderGUI() {
  // ImGui::ShowDemoWindow();
}

GLint CMyApp::ul(const char *uniformName) noexcept {
  GLuint programID = 0;

  // Kérdezzük le az aktuális programot!
  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
  glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint *>(&programID));
  // A program és a uniform név ismeretében kérdezzük le a location-t!
  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetUniformLocation.xhtml
  return glGetUniformLocation(programID, uniformName);
}

// https://wiki.libsdl.org/SDL2/SDL_KeyboardEvent
// https://wiki.libsdl.org/SDL2/SDL_Keysym
// https://wiki.libsdl.org/SDL2/SDL_Keycode
// https://wiki.libsdl.org/SDL2/SDL_Keymod

void CMyApp::KeyboardDown(const SDL_KeyboardEvent &key) {
  if (key.repeat == 0)  // Először lett megnyomva
  {
    if (key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_CTRL) {
      CleanShaders();
      InitShaders();
    }
    if (key.keysym.sym == SDLK_F1) {
      GLint polygonModeFrontAndBack[2] = {};
      // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
      glGetIntegerv(GL_POLYGON_MODE,
                    polygonModeFrontAndBack);  // Kérdezzük le a jelenlegi
      // polygon módot! Külön adja
      // a front és back módokat.
      GLenum polygonMode = (polygonModeFrontAndBack[0] != GL_FILL
                                ? GL_FILL
                                : GL_LINE);  // Váltogassuk FILL és LINE között!
      // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPolygonMode.xhtml
      glPolygonMode(GL_FRONT_AND_BACK,
                    polygonMode);  // Állítsuk be az újat!
    }
  }
  m_camera.KeyboardDown(key);
}

void CMyApp::KeyboardUp(const SDL_KeyboardEvent &key) {
  m_camera.KeyboardUp(key);
}

// https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent

void CMyApp::MouseMove(const SDL_MouseMotionEvent &mouse) {
  m_camera.MouseMove(mouse);
}

// https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent

void CMyApp::MouseDown(const SDL_MouseButtonEvent &mouse) {}

void CMyApp::MouseUp(const SDL_MouseButtonEvent &mouse) {}

// https://wiki.libsdl.org/SDL2/SDL_MouseWheelEvent

void CMyApp::MouseWheel(const SDL_MouseWheelEvent &wheel) {
  m_camera.MouseWheel(wheel);
}

// a két paraméterben az új ablakméret szélessége (_w) és magassága (_h)
// található
void CMyApp::Resize(int _w, int _h) {
  glViewport(0, 0, _w, _h);
  m_camera.Resize(_w, _h);
}