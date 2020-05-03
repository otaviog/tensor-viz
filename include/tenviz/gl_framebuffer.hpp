#pragma once

#include <memory>

#include "context_resource.hpp"
#include "gl_texture.hpp"

namespace tenviz {

enum FramebufferTarget {
  kRGBAUint8,
  kRGBAFloat,
  kRGBAInt32,
  kRGBUint8,
  kRGBFloat,
  kRGBInt32,
  kRUint8,
  kRFloat,
  kRInt32,
  kRUint32,
};

/**
 * OpenGL Framebuffer handler for offscreen rendering.
 *
 */
class GLFramebuffer : public IContextResource {
 public:
  /* Inititalizes the framebuffer under the current OpenGL's context.
   */
  GLFramebuffer();

  virtual ~GLFramebuffer();

  static std::shared_ptr<GLFramebuffer> Create();

  static void RegisterPybind(pybind11::module& m, IContextResource::PythonClassDef &base_class);

  GLFramebuffer(const GLFramebuffer& copy) = delete;
  GLFramebuffer& operator=(const GLFramebuffer& rhs) = delete;

  /**
   * Release the OpenGL resources.
   */
  void Release() override;

  /**
   *
   */
  void SetAttachment(int attach_num, FramebufferTarget target);

  /**
   * Sets the size of the frame buffer (width and height).
   * @size the buffer's width and height.
   */
  void SetSize(int width, int height);

  /**
   * Binds or unbinds the framebuffer from the current GL's context.
   *
   * @bind true to bind of false to unbind the instance's framebuffer.
   */
  void Bind(bool bind);

  std::map<int, std::shared_ptr<GLTexture>> GetAttachments();

  std::shared_ptr<GLTexture> GetDepth() { return depth_; }

  std::shared_ptr<GLTexture> GetAttachment(int loc);

 private:
  struct TargetEntry {
    std::shared_ptr<GLTexture> texture;
    FramebufferTarget target;
  };

  GLuint fbo_;
  std::map<int, TargetEntry> targets_;
  std::shared_ptr<GLTexture> depth_;
  int width_, height_;
  bool dirty_;
};
}  // namespace tenviz
