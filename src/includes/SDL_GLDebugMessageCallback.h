#pragma once

#include <GL/glew.h>

void GLAPIENTRY SDL_GLDebugMessageCallback(GLenum source, GLenum type,
                                           GLuint id, GLenum severity,
                                           GLsizei length, const GLchar* msg,
                                           const void* data);
