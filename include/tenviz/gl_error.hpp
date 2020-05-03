#pragma once

#include <string>

#include "error.hpp"
#include "gl_common.hpp"

#define GLCheckError() tenviz::_glCheckError(__FILE__, __LINE__)

namespace tenviz {

/* @return converted OpenGL error enum to its string version.
 */
const char *GetGLErrorString(GLenum gl_err);

inline void _glCheckError(const char *file, const int line) {
#ifndef NDEBUG
  const GLenum err = glGetError();

  if (err != GL_NO_ERROR && err != GL_FRAMEBUFFER_COMPLETE) {
    std::stringstream format;
    format << "GL call " << file << "(" << line
           << "): " << GetGLErrorString(err);
    throw Error(format);
  }
#endif
}

}  // namespace tenviz
