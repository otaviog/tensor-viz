#pragma once

#include <memory>
#include <mutex>
#include <set>

#include "eigen_common.hpp"
#include "gl_common.hpp"

#include "camera_manipulator.hpp"
#include "context_resource.hpp"

class GLFWwindow;

namespace pybind11 {
class module;
}

namespace tenviz {

class GLFramebuffer;
class Viewer;
class Scene;
class IContextResource;

/**
 * OpenGL context manager. Almost all operations envolve this class.
 *
 */
class Context {
 public:
  static void RegisterPybind(pybind11::module &m);

  /**
   * Set the context's base properties.
   *
   * @param width Context's window width.
   * @param height Context's window height.
   * @param profile Whatever the to enable OpenGL's core profile.
   */
  Context(int width, int height, bool profile = false);
  Context &operator=(const Context &copy) = delete;
  Context(const Context &copy) = delete;

  virtual ~Context() { Release(); }

  /**
   * Binds the context into the current thread. This will also
   * initialize the context's OpenGL and extension, if not done
   * before.
   *
   * Contexts are locked by a mutex. If two different context tries to
   * make current in the same thread, then a dead-lock will
   * occur. It's safe for the same context to make current more than
   * one time.
   */
  void MakeCurrent();

  /**
   * Unmake the context from the current thread.
   */
  void DetachCurrent();

  /**
   * @return Whatever the context is current.
   */
  bool IsCurrent() const;

  /**
   * Release all context resources.
   */
  void Release();

  /**
   * Render a scene into a framebuffer.
   *
   * @param projection OpenGL's projection matrix.
   *
   * @param camera OpenGL's view matrix.
   *
   * @param framebuffer The target framebuffer.
   *
   * @param scene The scene. All geometry must be loaded on the
   * context.
   *
   * @param width override the width for the current call.
   *
   * @param height override the height for the current call.
   */
  void Render(const Eigen::Matrix4f &projection, const Eigen::Matrix4f &camera,
              std::shared_ptr<GLFramebuffer> framebuffer,
              std::shared_ptr<Scene> scene, int width = -1, int height = -1);

  /**
   * Resize the context dimensions.
   *
   * @param width New width.
   * @param height New height.
   */
  void Resize(int width, int height);

  /**
   * Create a viewer window for a scene loaded into the context.
   *
   * @param scene Target scene. Nullptr will use all geometry of the
   * context.
   *
   * @param cam_manip The camera manipulator for the viewer.
   */
  std::shared_ptr<Viewer> CreateViewer(std::shared_ptr<Scene> scene,
                                       CameraManipulator cam_manip = kWASD);

  /**
   * Collect all loose IContextResource s.
   */
  void CollectGarbage();

  /**
   * @return The current width.
   */
  int get_width() const { return width_; }

  /**
   * @return The current height.
   */
  int get_height() const { return height_; }

  Eigen::Vector4f clear_color; /** Clear color property. */

 protected:
  /**
   * Initialize the context if not done before.
   */
  virtual void Initialize();

  int width_, height_;
  GLFWwindow *window_;

 private:
  /**
   * Register an OpenGL resource (classes that derive
   * IContextResource) into the context that is currently associated
   * to the thread.
   */
  static void RegisterResourceOnCurrent(
      std::shared_ptr<IContextResource> resource);

  void RenderFrameImpl(std::shared_ptr<Scene> scene,
                       const Eigen::Matrix4f &projection,
                       const Eigen::Matrix4f &camera);

  friend class Viewer;
  friend class IContextResource;

  std::set<std::shared_ptr<IContextResource>> resources_;

  std::mutex context_lock_;

  int viewer_count_;
  bool profile_;
};

/**
 * RAII for making a context current and detatching it.
 */
class ScopedCurrent {
 public:
  ScopedCurrent(Context &context) : context_(context) {
    context_.MakeCurrent();
  }

  ~ScopedCurrent() { context_.DetachCurrent(); }

 private:
  Context &context_;
};

}  // namespace tenviz
