#include "MyApp.h"
#include "ObjParser.h"
#include "ParametricSurfaceMesh.hpp"
#include "SDL_GLDebugMessageCallback.h"

#include <cassert>
#include <cmath>
#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <imgui.h>

CMyApp::CMyApp() {}

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
                  "shaders/Frag_PosCol.frag");

  InitSkyboxShaders();
}

void CMyApp::InitSkyboxShaders() {}

void CMyApp::CleanShaders() {
  glDeleteProgram(m_programID);
  glDeleteProgram(m_programAxesID);
  glDeleteProgram(m_programPointID);

  CleanSkyboxShaders();
}

void CMyApp::CleanSkyboxShaders() {}

// Egy SIZE_X és SIZE_Y méretű sík
struct ParametricSurface {
  glm::vec3 GetPos(float u, float v) const noexcept {

    return glm::vec3(u, v, 0);
  }

  glm::vec3 GetNorm(float u, float v) const noexcept {
    return glm::vec3(u, v, 0);
  }

  glm::vec2 GetTex(float u, float v) const noexcept { return glm::vec2(u, v); }
};

struct Capsule {
public:
  static constexpr float radius = 0.1f; // A henger sugara
  static constexpr float height = 1.0f; // Kapszula magassága
  // A pontok aránya:
  // cutoff - félgömb, maradék henger, cutoff - félgömb
  static constexpr float spherecutoff = 0.2;
  static_assert(spherecutoff > 0);
  static_assert(height > 0);
  static_assert(spherecutoff < 0.5);
  glm::vec3 GetPos(float u, float v) const noexcept {

    return glm::vec3(u, v, 0);
  }

  glm::vec3 GetNorm(float u, float v) const noexcept {
    return glm::vec3(u, v, 0);
  }

  glm::vec2 GetTex(float u, float v) const noexcept { return glm::vec2(u, v); }
};

// henger
struct Cylinder {
public:
  static constexpr float radius = 0.1f; // A henger sugara
  static constexpr float height = 1.0f; // Kapszula magassága
  // A pontok aránya:
  // cutoff - félgömb, maradék henger, cutoff - félgömb
  static_assert(height > 0);
  static_assert(radius > 0);
  glm::vec3 GetPos(float u, float v) const noexcept {

    return glm::vec3(u, v, 0);
  }

  glm::vec3 GetNorm(float u, float v) const noexcept {
    return glm::vec3(u, v, 0);
  }

  glm::vec2 GetTex(float u, float v) const noexcept { return glm::vec2(u, v); }
};
// Kúp
struct Cone {
public:
  static constexpr float radius = 0.1f; // A henger sugara
  static constexpr float height = 0.4f; // Kapszula magassága
  // A pontok aránya:
  // cutoff - félgömb, maradék henger, cutoff - félgömb
  static_assert(height > 0);
  static_assert(radius > 0);
  glm::vec3 GetPos(float u, float v) const noexcept {

    return glm::vec3(u, v, 0);
  }

  glm::vec3 GetNorm(float u, float v) const noexcept {
    return glm::vec3(u, v, 0);
  }

  glm::vec2 GetTex(float u, float v) const noexcept { return glm::vec2(u, v); }
};
// Ez biztosan része lesz a kezdő projektnek.
MeshObject<Vertex> createBoxMesh() {
  return MeshObject<Vertex>{
      {// hátsó lap
       // Front face
       {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
       {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
       {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
       {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
       // Back face
       {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
       {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
       {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
       {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
       // Top face
       {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
       {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
       {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
       {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
       // Bottom face
       {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
       {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
       {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
       {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
       // Right face
       {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
       {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
       {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
       {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
       // Left face
       {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
       {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
       {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
       {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}},
      {{
          0,  1,  2, // first triangle of the front face
          2,  3,  0, // second triangle of the front face

          6,  5,  4, // first triangle of the back face
          4,  7,  6, // second triangle of the back face

          10, 9,  8,  // first triangle of the top face
          8,  11, 10, // second triangle of the top face

          12, 13, 14, // first triangle of the bottom face
          14, 15, 12, // second triangle of the bottom face

          16, 17, 18, // first triangle of the right face
          18, 19, 16, // second triangle of the right face

          20, 21, 22, // first triangle of the left face
          22, 23, 20  // second triangle of the left face
      }}};
}
void CMyApp::InitGeometry() {
  const std::initializer_list<VertexAttributeDescriptor> vertexAttribList = {
      {0, offsetof(Vertex, position), 3, GL_FLOAT},
      {1, offsetof(Vertex, normal), 3, GL_FLOAT},
      {2, offsetof(Vertex, texcoord), 2, GL_FLOAT},
  };

  // Suzanne

  MeshObject<Vertex> suzanneMeshCPU = ObjParser::parse("Assets/Suzanne.obj");
  // https://free3d.com/3d-model/hat-v1--210279.html
  m_SuzanneGPU = CreateGLObjectFromMesh(suzanneMeshCPU, vertexAttribList);

  // Skybox
  InitSkyboxGeometry();
}

void CMyApp::CleanGeometry() {
  CleanOGLObject(m_SuzanneGPU);
  CleanSkyboxGeometry();
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

  m_SkyboxGPU = CreateGLObjectFromMesh(
      skyboxCPU, {{0, offsetof(glm::vec3, x), 3, GL_FLOAT}});
}

void CMyApp::CleanSkyboxGeometry() { CleanOGLObject(m_SkyboxGPU); }

void CMyApp::InitTextures() {
  // diffuse texture

  glGenTextures(1, &m_SuzanneTextureID);
  TextureFromFile(m_SuzanneTextureID, "Assets/wood.jpg");
  SetupTextureSampling(GL_TEXTURE_2D, m_SuzanneTextureID);
  InitSkyboxTextures();
}

void CMyApp::CleanTextures() {
  glDeleteTextures(1, &m_SuzanneTextureID);

  CleanSkyboxTextures();
}

void CMyApp::InitSkyboxTextures() {
  // skybox texture

  glGenTextures(1, &m_skyboxTextureID);
  TextureFromFile(m_skyboxTextureID, "Assets/lab_xpos.png", GL_TEXTURE_CUBE_MAP,
                  GL_TEXTURE_CUBE_MAP_POSITIVE_X);
  TextureFromFile(m_skyboxTextureID, "Assets/lab_xneg.png", GL_TEXTURE_CUBE_MAP,
                  GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
  TextureFromFile(m_skyboxTextureID, "Assets/lab_ypos.png", GL_TEXTURE_CUBE_MAP,
                  GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
  TextureFromFile(m_skyboxTextureID, "Assets/lab_yneg.png", GL_TEXTURE_CUBE_MAP,
                  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
  TextureFromFile(m_skyboxTextureID, "Assets/lab_zpos.png", GL_TEXTURE_CUBE_MAP,
                  GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
  TextureFromFile(m_skyboxTextureID, "Assets/lab_zneg.png", GL_TEXTURE_CUBE_MAP,
                  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
  SetupTextureSampling(GL_TEXTURE_CUBE_MAP, m_skyboxTextureID, false);

  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void CMyApp::CleanSkyboxTextures() { glDeleteTextures(1, &m_skyboxTextureID); }

bool CMyApp::Init() {
  SetupDebugCallback();

  // törlési szín legyen kékes
  glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

  // Nem minden driver támogatja a vonalak és pontok vastagabb
  // megjelenítését, ezért lekérdezzük, hogy támogatott-e a
  // GL_LINE_WIDTH_RANGE és GL_POINT_SIZE_RANGE tokenek.
  {
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glPointSize.xhtml
    GLfloat pointSizeRange[2] = {0.0f, 0.0f};
    glGetFloatv(GL_POINT_SIZE_RANGE,
                pointSizeRange); // lekérdezzük a támogatott pontméretek
    // tartományát
    glPointSize(std::min(16.0f, pointSizeRange[1])); // nagyobb pontok
  }

  {
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glLineWidth.xhtml
    GLfloat lineWidthRange[2] = {0.0f, 0.0f};
    glGetFloatv(GL_LINE_WIDTH_RANGE,
                lineWidthRange); // lekérdezzük a támogatott
    // vonalvastagság tartományát
    glLineWidth(std::min(4.0f, lineWidthRange[1])); // vastagabb vonalak
  }

  InitShaders();
  InitGeometry();
  InitTextures();

  //
  // egyéb inicializálás
  //

  glEnable(GL_CULL_FACE); // kapcsoljuk be a hátrafelé néző lapok eldobását
  glCullFace(GL_BACK);    // GL_BACK: a kamerától "elfelé" néző lapok,
  // GL_FRONT: a kamera felé néző lapok

  glEnable(GL_DEPTH_TEST); // mélységi teszt bekapcsolása (takarás)

  // kamera
  m_camera.SetView(
      glm::vec3(0.0, 7.0, 7.0),  // honnan nézzük a színteret	   - eye
      glm::vec3(0.0, 0.0, 0.0),  // a színtér melyik pontját nézzük - at
      glm::vec3(0.0, 1.0, 0.0)); // felfelé mutató irány a világban - up

  std::vector<std::pair<int, int>> ControlPointsCoordinates = {
      {4, 3}, {5, 3}, {6, 3}, {7, 3}, {7, 4}, {7, 5}, {7, 6}, {7, 7}};

  // Directly initialize m_walls using list initialization with pairs
  std::vector<std::pair<int, int>> InnerWallCoordinates = {
      {2, 2}, {3, 2}, {5, 2}, {6, 2}, {2, 3}, {6, 3}, {7, 3},
      {7, 4}, {2, 5}, {4, 5}, {5, 5}, {7, 5}, {2, 6}, {4, 6},
      {7, 6}, {2, 7}, {6, 7}, {7, 7}, {2, 8}, {4, 8}, {6, 8}};
  std::vector<std::pair<int, int>> OuterWallCoordinates;
  OuterWallCoordinates.reserve(SIZE_X * 2 + SIZE_Y * 2 - 4);

  for (int i = 0; i < SIZE_X; i++) {
    OuterWallCoordinates.push_back({i, 0});
    OuterWallCoordinates.push_back({i, SIZE_Y - 1});
  }
  for (int j = 1; j < SIZE_Y - 1; j++) {
    OuterWallCoordinates.push_back({0, j});
    OuterWallCoordinates.push_back({SIZE_X - 1, j});
  }
  std::pair<int, int> DinamiteCoordinates = {5, 4};
  // TODO: Turn these into glm::vec3 coordinates.

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

void CMyApp::Render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // NOTE: Leírásban megadott
  static const glm::mat4 TableOffset = glm::translate(glm::vec3(0, -11.7, 0));

  // NOTE: Az objektumok "normalizáló" transzformációi
  //
  // ------------------------------------------------------------------------------------
  // NOTE: asztal
  static const glm::mat4 tableTowardsY =
      glm::rotate(glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
  static const glm::mat4 shrinkTable = glm::scale(glm::vec3(0.3f));
  static const glm::mat4 TableCenter =
      glm::translate(glm::vec3(0.f, -11, 0.0f));

  static const glm::mat4 TableBaseTransform =
      TableCenter * tableTowardsY * shrinkTable;
  // NOTE: Suzanne

  static const glm::mat4 suzanneScaling = glm::scale(glm::vec3(0.35f));
  // A Suzanne alapállásban a Z tengelyre néz, de nekünk az X tengelyre
  // kell, ezért elforgatjuk
  static const glm::mat4 suzanneTowardX =
      glm::rotate(glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
  static const glm::mat4 SuzanneBaseTransform = suzanneScaling * suzanneTowardX;
  // NOTE: A sapka

  static const glm::mat4 hatTowardsX =
      glm::rotate(glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
  static const glm::mat4 shrinkHat = glm::scale(glm::vec3(0.025f));
  static const glm::mat4 hatCenter = glm::translate(glm::vec3(-3.0, 0.0f, 20));
  static const glm::mat4 HatBaseTransform = hatTowardsX * shrinkHat * hatCenter;
  static const glm::mat4 HatOffsetFromSuzanne =
      glm::translate(glm::vec3(0.0f, 0.23f, 0.0f));

  // ------------------------------------------------------------------------------------

  glm::mat4 matWorld = glm::identity<glm::mat4>();

  const static float dinamiteappear = 2.5;
  const static float dinamiteexplode = 4.5;
  const static float explosionlength = 2;

  const glm::vec4 lightPos[1] = {m_lightPos};
  const glm::vec3 La[1] = {m_La};
  const glm::vec3 Ld[1] = {m_Ld};
  const glm::vec3 Ls[1] = {m_Ls};

  //
  // Suzanne
  //
  glm::mat4 suzanneRot;
  {
    glBindVertexArray(m_SuzanneGPU.vaoID);

    // - Textúrák beállítása, minden egységre külön
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_SuzanneTextureID);

    glUseProgram(m_programID);

    // - uniform parameterek beállítása

    glm::vec3 suzanneForward = EvaluatePathTangent(); // Merre nézzen a Suzanne?
    glm::vec3 suzanneRight = glm::normalize(
        glm::cross(suzanneForward, glm::vec3(0.0, 1.0, 0.0))); // Jobbra nézése
    glm::vec3 suzanneUp =
        glm::cross(suzanneRight, suzanneForward); // Felfelé nézése

    // A három vektorból álló bázisvektorokat egy mátrixba rendezzük, hogy
    // tudjuk velük forgatni a Suzanne-t
    suzanneRot = glm::mat4(1.0f);
    suzanneRot[0] = glm::vec4(suzanneForward, 0.0f);
    suzanneRot[1] = glm::vec4(suzanneUp, 0.0f);
    suzanneRot[2] = glm::vec4(suzanneRight, 0.0f);

    matWorld = glm::translate(EvaluatePathPosition()) * suzanneRot *
               suzanneScaling * suzanneTowardX;

    glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(matWorld));
    glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE,
                       glm::value_ptr(glm::transpose(glm::inverse(matWorld))));

    glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE,
                       glm::value_ptr(m_camera.GetViewProj()));

    // - Fényforrások beállítása
    glUniform3fv(ul("cameraPos"), 1, glm::value_ptr(m_camera.GetEye()));
    glUniform4fv(ul("lightPos"), 1, glm::value_ptr(lightPos[0]));

    glUniform3fv(ul("La"), 1, glm::value_ptr(La[0]));
    glUniform3fv(ul("Ld"), 1, glm::value_ptr(Ld[0]));
    glUniform3fv(ul("Ls"), 1, glm::value_ptr(Ls[0]));

    glUniform1f(ul("lightConstantAttenuation"), m_lightConstantAttenuation);
    glUniform1f(ul("lightLinearAttenuation"), m_lightLinearAttenuation);
    glUniform1f(ul("lightQuadraticAttenuation"), m_lightQuadraticAttenuation);

    // - Anyagjellemzők beállítása
    glUniform3fv(ul("Ka"), 1, glm::value_ptr(m_Ka));
    glUniform3fv(ul("Kd"), 1, glm::value_ptr(m_Kd));
    glUniform3fv(ul("Ks"), 1, glm::value_ptr(m_Ks));

    glUniform1f(ul("Shininess"), m_Shininess);

    // - textúraegységek beállítása
    glUniform1i(ul("texImage"), 0);

    glDrawElements(GL_TRIANGLES, m_SuzanneGPU.count, GL_UNSIGNED_INT, nullptr);
    glUseProgram(0);
  }

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
  //
  // skybox
  //
  {
    // - VAO
    glBindVertexArray(m_SkyboxGPU.vaoID);

    // - Textura
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTextureID);

    // - Program
    glUseProgram(m_programSkyboxID);

    // - uniform parameterek
    glUniformMatrix4fv(ul("world"), 1, GL_FALSE,
                       glm::value_ptr(glm::translate(m_camera.GetEye())));
    glUniformMatrix4fv(ul("viewProj"), 1, GL_FALSE,
                       glm::value_ptr(m_camera.GetViewProj()));

    // - textúraegységek beállítása
    glUniform1i(ul("skyboxTexture"), 0);

    // mentsük el az előző Z-test eredményt, azaz azt a relációt, ami
    // alapján update-eljük a pixelt.
    GLint prevDepthFnc;
    glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFnc);

    // most kisebb-egyenlőt használjunk, mert mindent kitolunk a távoli
    // vágósíkokra
    glDepthFunc(GL_LEQUAL);

    // - Rajzolas
    glDrawElements(GL_TRIANGLES, m_SkyboxGPU.count, GL_UNSIGNED_INT, nullptr);

    glDepthFunc(prevDepthFnc);
  }
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
  if (ImGui::Begin("Lighting settings")) {
    ImGui::InputFloat("Shininess", &m_Shininess, 0.1f, 1.0f, "%.1f");
    static float Laf = 1.0f;
    static float Ldf = 1.0f;
    static float Lsf = 1.0f;
    if (ImGui::SliderFloat("La", &Laf, 0.0f, 1.0f)) {
      m_La = glm::vec3(Laf);
    }
    if (ImGui::SliderFloat("Ld", &Ldf, 0.0f, 1.0f)) {
      m_Ld = glm::vec3(Ldf);
    }
    if (ImGui::SliderFloat("Ls", &Lsf, 0.0f, 1.0f)) {
      m_Ls = glm::vec3(Lsf);
    }

    {
      static glm::vec2 lightPosXZ = glm::vec2(0.0f);
      lightPosXZ = glm::vec2(m_lightPos.x, m_lightPos.z);
      if (ImGui::SliderFloat2("Light Position XZ", glm::value_ptr(lightPosXZ),
                              -1.0f, 1.0f)) {
        float lightPosL2 =
            lightPosXZ.x * lightPosXZ.x + lightPosXZ.y * lightPosXZ.y;
        if (lightPosL2 > 1.0f) // Ha kívülre esne a körön, akkor normalizáljuk
        {
          lightPosXZ /= sqrtf(lightPosL2);
          lightPosL2 = 1.0f;
        }

        m_lightPos.x = lightPosXZ.x;
        m_lightPos.z = lightPosXZ.y;
        m_lightPos.y = sqrtf(1.0f - lightPosL2);
      }
      ImGui::LabelText("Light Position Y", "%f", m_lightPos.y);
    }
  }
  ImGui::End();

  if (ImGui::Begin("Animation")) {
    // A paramétert szabályozó csúszka
    ImGui::SliderFloat("Parameter", &m_currentParam, 0,
                       (float)(m_controlPoints.size() - 1));
  }
  ImGui::End();
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
  if (key.repeat == 0) // Először lett megnyomva
  {
    if (key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_CTRL) {
      CleanShaders();
      InitShaders();
    }
    if (key.keysym.sym == SDLK_F1) {
      GLint polygonModeFrontAndBack[2] = {};
      // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
      glGetIntegerv(GL_POLYGON_MODE,
                    polygonModeFrontAndBack); // Kérdezzük le a jelenlegi
      // polygon módot! Külön adja
      // a front és back módokat.
      GLenum polygonMode = (polygonModeFrontAndBack[0] != GL_FILL
                                ? GL_FILL
                                : GL_LINE); // Váltogassuk FILL és LINE között!
      // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPolygonMode.xhtml
      glPolygonMode(GL_FRONT_AND_BACK,
                    polygonMode); // Állítsuk be az újat!
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

// Pozíció kiszámítása a kontrollpontok alapján
glm::vec3 CMyApp::EvaluatePathPosition() const {
  if (m_controlPoints.size() == 0) // Ha nincs pont, akkor visszaadjuk az origót
    return glm::vec3(0);

  const int interval =
      (const int)m_currentParam; // Melyik két pont között vagyunk?

  if (interval < 0) // Ha a paraméter negatív, akkor a kezdőpontot adjuk vissza
    return m_controlPoints[0];

  if (interval >=
      m_controlPoints.size() - 1) // Ha a paraméter nagyobb, mint a pontok
    // száma, akkor az utolsó pontot adjuk vissza
    return m_controlPoints[m_controlPoints.size() - 1];

  float localT =
      m_currentParam -
      interval; // A paramétert normalizáljuk az aktuális intervallumra

  return glm::mix(m_controlPoints[interval], m_controlPoints[interval + 1],
                  localT); // Lineárisan interpolálunk a két kontrollpont között
}

// Tangens kiszámítása a kontrollpontok alapján
glm::vec3 CMyApp::EvaluatePathTangent() const {
  if (m_controlPoints.size() < 2) // Ha nincs elég pont az interpolációhoy,
    // akkor visszaadjuk az x tengelyt
    return glm::vec3(1.0, 0.0, 0.0);

  int interval = (int)m_currentParam; // Melyik két pont között vagyunk?

  if (interval < 0) // Ha a paraméter negatív, akkor a kezdő intervallumot
    // adjuk vissza
    interval = 0;

  if (interval >= m_controlPoints.size() -
                      1) // Ha a paraméter nagyobb, mint az intervallumok
    // száma, akkor az utolsót adjuk vissza
    interval = static_cast<int>(m_controlPoints.size() - 2);

  return glm::normalize(m_controlPoints[interval + 1] -
                        m_controlPoints[interval]);
}
void CMyApp::CalculateFallingStateStart() {}
