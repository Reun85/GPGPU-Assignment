#pragma once

#include <SDL2/SDL.h>

#include <glm/glm.hpp>

class Camera {
 public:
  Camera();
  Camera(glm::vec3 _eye, glm::vec3 _at, glm::vec3 _worldup);

  ~Camera();

  inline glm::vec3 GetEye() const { return m_eye; }
  inline glm::vec3 GetAt() const { return m_at; }
  inline glm::vec3 GetWorldUp() const { return m_worldUp; }

  inline glm::mat4 GetViewMatrix() const { return m_viewMatrix; }
  inline glm::mat4 GetProj() const { return m_matProj; }
  inline glm::mat4 GetViewProj() const { return m_matViewProj; }

  bool Update(float _deltaTime);

  void SetView(glm::vec3 _eye, glm::vec3 _at, glm::vec3 _up);
  void LookAt(glm::vec3 _at);

  inline float GetAngle() const { return m_angle; }
  void SetAngle(const float _angle) noexcept;
  inline float GetAspect() const { return m_aspect; }
  void SetAspect(const float _aspect) noexcept;
  inline float GetZNear() const { return m_zNear; }
  void SetZNear(const float _zn) noexcept;
  inline float GetZFar() const { return m_zFar; }
  void SetZFar(const float _zf) noexcept;

  void SetProj(float _angle, float _aspect, float _zn, float _zf);

  void SetSpeed(float _val);
  void Resize(int _w, int _h);

  void KeyboardDown(const SDL_KeyboardEvent& key);
  void KeyboardUp(const SDL_KeyboardEvent& key);
  void MouseMove(const SDL_MouseMotionEvent& mouse);
  void MouseWheel(const SDL_MouseWheelEvent& wheel);
  
  void SetFirstPerson(bool _firstperson);
  bool GetFirstPerson() const { return firstperson; }
 private:
  bool firstperson = false;


  // Updates the UV.
  void UpdateUV(float du, float dv);

  // Updates the distance.
  void UpdateDistance(float dDistance);

  // Updates the underlying parameters.
  void UpdateParams();

  //  The traversal speed of the camera
  float m_speed = 16.0f;

  bool m_slow = false;

  // The camera position.
  glm::vec3 m_eye;

  // The vector pointing upwards
  glm::vec3 m_worldUp;

  // The camera look at point.
  glm::vec3 m_at;

  // The u spherical coordinate of the spherical coordinate pair (u,v) denoting
  // the current viewing direction from the view position m_eye.
  float m_u;

  // The v spherical coordinate of the spherical coordinate pair (u,v) denoting
  // the current viewing direction from the view position m_eye.
  float m_v;

  // The distance of the look at point from the camera.
  float m_distance;

  // The unit vector pointing towards the viewing direction.
  glm::vec3 m_forward;

  // The unit vector pointing to the 'right'
  glm::vec3 m_right;

  // The vector pointing upwards
  glm::vec3 m_up;

  float m_goForward = 0.0f;
  float m_goRight = 0.0f;
  float m_goUp = 0.0f;

  // view matrix needs recomputation
  bool m_viewDirty = true;

  // The view matrix of the camera
  glm::mat4 m_viewMatrix;

  // projection parameters
  float m_zNear = 0.01f;
  float m_zFar = 1000.0f;

  float m_angle = glm::radians(27.0f);
  float m_aspect = 640.0f / 480.0f;

  // projection matrix needs recomputation
  bool m_projectionDirty = true;

  // projection matrix
  glm::mat4 m_matProj;

  // Combined view-projection matrix of the camera
  glm::mat4 m_matViewProj;
};
