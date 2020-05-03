#include "gl_framebuffer.hpp"

#include <vector>

#include "gl_error.hpp"
#include "scoped_bind.hpp"

using namespace std;

namespace tenviz {

namespace {
inline void glCheckError(GLenum err, const char *file, const int line) {
#ifndef NDEBUG
  if (err != GL_NO_ERROR && err != GL_FRAMEBUFFER_COMPLETE) {
    stringstream format;
    format << "GL call " << file << "(" << line
           << "): " << GetGLErrorString(err);
    throw Error(format);
  }
#endif
}
}  // namespace

void GLFramebuffer::RegisterPybind(
    pybind11::module &m, IContextResource::PythonClassDef &base_class) {
  py::enum_<FramebufferTarget>(m, "FramebufferTarget")
      .value("RGBAUint8", FramebufferTarget::kRGBAUint8)
      .value("RGBAFloat", FramebufferTarget::kRGBAFloat)
      .value("RGBAInt32", FramebufferTarget::kRGBAInt32)
      .value("RGBUint8", FramebufferTarget::kRGBUint8)
      .value("RGBFloat", FramebufferTarget::kRGBFloat)
      .value("RGBInt32", FramebufferTarget::kRGBInt32)
      .value("RInt32", FramebufferTarget::kRInt32)
      .value("RUint32", FramebufferTarget::kRUint32)
      .value("RFloat", FramebufferTarget::kRFloat)
      .value("RUint8", FramebufferTarget::kRUint8)
      .export_values();

  py::class_<GLFramebuffer, shared_ptr<GLFramebuffer>>(m, "Framebuffer")
      .def("set_attachment", &GLFramebuffer::SetAttachment)
      .def("set_size", &GLFramebuffer::SetSize)
      .def("get_attachs", &GLFramebuffer::GetAttachments)
      .def("get_depth", &GLFramebuffer::GetDepth)
      .def("__getitem__", &GLFramebuffer::GetAttachment);

  m.def("create_framebuffer", &GLFramebuffer::Create);
}

GLFramebuffer::GLFramebuffer() {
  width_ = height_ = 0;
  dirty_ = true;
  glGenFramebuffers(1, &fbo_);
  GLCheckError();

  depth_ = GLTexture::Create(TexTarget::kRectangle);
}

GLFramebuffer::~GLFramebuffer() { Release(); }

std::shared_ptr<GLFramebuffer> GLFramebuffer::Create() {
  auto framebuffer = make_shared<GLFramebuffer>();
  IContextResource::RegisterResourceOnCurrent(framebuffer);
  return framebuffer;
}

void GLFramebuffer::Release() {
  if (fbo_ == -1) return;

  glDeleteFramebuffers(1, &fbo_);
  GLCheckError();
  fbo_ = -1;
}

void GLFramebuffer::SetAttachment(int attach_num, FramebufferTarget target) {
  if (targets_.count(attach_num)) {
    if (targets_[attach_num].target != target) {
      targets_[attach_num].target = target;
      dirty_ = true;
    }

    return;
  }

  TargetEntry entry;
  entry.texture = GLTexture::Create(TexTarget::kRectangle);
  entry.target = target;

  targets_[attach_num] = entry;
  dirty_ = true;
}

void GLFramebuffer::SetSize(int width, int height) {
  if (width_ == width && height_ == height && !dirty_) {
    return;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  GLCheckError();

  for (auto item : targets_) {
    const int attach_num = item.first;
    auto texture = item.second.texture;
    auto target = item.second.target;

    texture->SetParameters(GLTextureParameters(
        GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false));

    switch (target) {
      case kRGBAUint8:
        texture->TexImage(GL_RGBA8, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
                          nullptr);
        break;
      case kRGBAFloat:
        texture->TexImage(GL_RGBA32F, width, height, GL_RGBA, GL_FLOAT,
                          nullptr);
        break;
      case kRGBAInt32:
        texture->TexImage(GL_RGBA32I, width, height, GL_RGBA_INTEGER, GL_INT,
                          nullptr);
        break;

      case kRGBUint8:
        texture->TexImage(GL_RGB8, width, height, GL_RGB, GL_UNSIGNED_BYTE,
                          nullptr);
        break;
      case kRGBFloat:
        texture->TexImage(GL_RGB32F, width, height, GL_RGB, GL_FLOAT, nullptr);
        break;
      case kRGBInt32:
        texture->TexImage(GL_RGB32I, width, height, GL_RGB_INTEGER, GL_INT,
                          nullptr);
        break;

      case kRInt32:
        texture->TexImage(GL_LUMINANCE32I_EXT, width, height,
                          GL_LUMINANCE_INTEGER_EXT, GL_INT, nullptr);
        break;
      case kRUint32:
        texture->TexImage(GL_R32UI, width, height, GL_RED_INTEGER,
                          GL_UNSIGNED_INT, nullptr);
        break;

      case kRUint8:
        texture->TexImage(GL_R8UI, width, height, GL_RED_INTEGER,
                          GL_UNSIGNED_BYTE, nullptr);
        break;
      case kRFloat:
        texture->TexImage(GL_R32F, width, height, GL_RED, GL_FLOAT, nullptr);
        break;
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attach_num,
                           GL_TEXTURE_RECTANGLE, texture->get_id(), 0);
    GLCheckError();
  }

  {
    depth_->SetParameters(GLTextureParameters(
        GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false));
    depth_->TexImage(GL_DEPTH_COMPONENT24, width, height, GL_DEPTH_COMPONENT,
                     GL_FLOAT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_RECTANGLE, depth_->get_id(), 0);
  }

  glCheckError(glCheckFramebufferStatus(GL_FRAMEBUFFER), __FILE__, __LINE__);
  GLCheckError();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  GLCheckError();

  width_ = width;
  height_ = height;
  dirty_ = false;
}

void GLFramebuffer::Bind(bool bind) {
  if (!bind) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLCheckError();
    return;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  GLCheckError();

  vector<GLenum> draw_buffers;
  draw_buffers.reserve(targets_.size());
  for (const auto &item : targets_) {
    const int attach_num = item.first;
    draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + attach_num);
    GLCheckError();
  }

  glDrawBuffers(draw_buffers.size(), &draw_buffers[0]);
  GLCheckError();

  glCheckError(glCheckFramebufferStatus(GL_FRAMEBUFFER), __FILE__, __LINE__);
  GLCheckError();
}

map<int, shared_ptr<GLTexture>> GLFramebuffer::GetAttachments() {
  map<int, shared_ptr<GLTexture>> attachs;

  for (auto item : targets_) {
    attachs[item.first] = item.second.texture;
  }

  return attachs;
}

shared_ptr<GLTexture> GLFramebuffer::GetAttachment(int loc) {
  auto iter = targets_.find(loc);
  if (iter == targets_.end()) {
    stringstream stream;
    stream << "No framebuffer number " << loc;
    throw Error(stream);
  }

  return iter->second.texture;
}

}  // namespace tenviz
