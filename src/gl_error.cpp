#include "gl_error.hpp"

namespace tenviz {
const char* GetGLErrorString(GLenum gl_err) {
  switch (gl_err) {
    case GL_NO_ERROR:
      return "No error has been recorded. The value of this symbolic constant "
             "is guaranteed to be 0.";
    case GL_INVALID_ENUM:
      return "An unacceptable value is specified for an enumerated argument. "
             "The offending command is ignored and has no other side effect "
             "than to set the error flag.";
    case GL_INVALID_VALUE:
      return "A numeric argument is out of range. The offending command is "
             "ignored and has no other side effect than to set the error flag.";
    case GL_INVALID_OPERATION:
      return "The specified operation is not allowed in the current state. The "
             "offending command is ignored and has no other side effect than "
             "to set the error flag.";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return "The framebuffer object is not complete. The offending command is "
             "ignored and has no other side effect than to set the error flag.";
    case GL_OUT_OF_MEMORY:
      return "There is not enough memory left to execute the command. The "
             "state of the GL is undefined, except for the state of the error "
             "flags, after this error is recorded.";
#ifndef GL_ES
    case GL_STACK_UNDERFLOW:
      return "An attempt has been made to perform an operation that would "
             "cause an internal stack to underflow.";
    case GL_STACK_OVERFLOW:
#endif
      return "An attempt has been made to perform an operation that would "
             "cause an internal stack to overflow.";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      return "Any of the framebuffer attachment points are framebuffer "
             "incomplete.";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      return "The framebuffer does not have at least one image attached to it.";
#ifndef GL_ES
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      return "The value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE​ is "
             "GL_NONE​ for any color attachment point(s) named by "
             "GL_DRAWBUFFERi​.";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      return "GL_READ_BUFFER​ is not GL_NONE​ and the value of "
             "GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE​ is GL_NONE​ for the "
             "color attachment point named by GL_READ_BUFFER​.";
    case GL_FRAMEBUFFER_UNSUPPORTED:
      return "The combination of internal formats of the attached images "
             "violates an implementation-dependent set of restrictions.";
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      return "The value of GL_RENDERBUFFER_SAMPLES​ is not the same for all "
             "attached renderbuffers; if the value of GL_TEXTURE_SAMPLES​ is "
             "the not same for all attached textures; or, if the attached "
             "images are a mix of renderbuffers and textures, the value of "
             "GL_RENDERBUFFER_SAMPLES​ does not match the value of "
             "GL_TEXTURE_SAMPLES​.";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      return "Any framebuffer attachment is layered, and any populated "
             "attachment is not layered, or if all populated color attachments "
             "are not from textures of the same target.";
#endif
    default:
      return "Undefined error";
  }
}
}  // namespace tenviz
