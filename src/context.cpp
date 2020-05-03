#include "context.hpp"

#include <pybind11/eigen.h>
#include <torch/csrc/utils/pybind.h>

#include "gl_error.hpp"
#include "gl_framebuffer.hpp"
#include "scene.hpp"
#include "scoped_bind.hpp"
#include "trackball_camera_manipulator.hpp"
#include "viewer.hpp"
#include "wasd_camera_manipulator.hpp"

using namespace std;

namespace tenviz {
namespace {

thread_local Context *g_current = nullptr;
thread_local int make_current_count;

int g_glfw_use_count = 0;  // TODO: use memory safe multithread.

void MyGLFWErrorCallback(int, const char *message) {
  stringstream format;
  format << "GLFW error: " << message;
  throw Error(format);
}

bool SafeGLFWInit() {
  if (g_glfw_use_count == 0) {
    if (!glfwInit()) return false;

    glfwSetErrorCallback(MyGLFWErrorCallback);
  }
  g_glfw_use_count += 1;
  return true;
}

void SafeGLFWTerminate() {
  if (g_glfw_use_count == 1) {
    glfwTerminate();
  }
  g_glfw_use_count -= 1;
}
}  // namespace

Context::Context(int width, int height, bool profile)
    : width_(width), height_(height), clear_color(0.32, 0.34, 0.87, 1) {
  window_ = nullptr;
  viewer_count_ = 0;
  profile_ = profile;
}

void Context::RegisterPybind(pybind11::module &m) {
  pybind11::enum_<CameraManipulator>(m, "CameraManipulator")
      .value("WASD", CameraManipulator::kWASD)
      .value("TrackBall", CameraManipulator::kTrackBall)
      .export_values();

  pybind11::class_<Context>(m, "Context")
      .def(py::init<int, int>())
      .def("_make_current", &Context::MakeCurrent)
      .def("_detach_current", &Context::DetachCurrent)
      .def("is_current", &Context::IsCurrent)
      .def("render", &Context::Render)
      .def("viewer", &Context::CreateViewer, py::arg("scene") = nullptr,
           py::arg("manip") = CameraManipulator::kTrackBall)
      .def("collect_garbage", &Context::CollectGarbage)
      .def("resize", &Context::Resize)
      .def_property("width", &Context::get_width, nullptr)
      .def_property("height", &Context::get_height, nullptr)
      .def_readwrite("clear_color", &Context::clear_color);
}

void Context::Initialize() {
  if (!SafeGLFWInit()) {
    throw Error("Failed to initialize OpenGL extensions");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  if (profile_) {
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  }

  glfwWindowHint(GLFW_VISIBLE, 0);
  glfwWindowHint(GLFW_SAMPLES, 4);

  window_ = glfwCreateWindow(width_, height_, "Viewer", NULL, NULL);
  if (!window_) {
    SafeGLFWTerminate();
    return;
  }
  glfwSetWindowUserPointer(window_, this);

  glfwMakeContextCurrent(window_);
  glfwSwapInterval(0);
  if (glewInit() != GLEW_OK) {
    SafeGLFWTerminate();
    return;
  }

  glPixelStorei(GL_PACK_ALIGNMENT, 1);  // For Texuture reads
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);
  glfwMakeContextCurrent(nullptr);
}

void Context::Release() {
  if (window_ == nullptr) {
    return;
  }

  {
    ScopedCurrent curr(*this);

    for (auto resource : resources_) {
      resource->Release();
    }
    resources_.clear();
  }

  glfwDestroyWindow(window_);
  SafeGLFWTerminate();
  window_ = nullptr;
}

void Context::CollectGarbage() {
  ScopedCurrent curr(*this);
  for (auto iter = resources_.begin(); iter != resources_.end();) {
    auto resource = *iter;
    if (resource.use_count() == 1) {
      resource->Release();
      iter = resources_.erase(iter);
    } else {
      ++iter;
    }
  }
}

void Context::MakeCurrent() {
  if (!window_) {
    Initialize();
  }
  if (g_current == this) {
    make_current_count += 1;
    return;
  }

  context_lock_.lock();
  g_current = this;
  glfwMakeContextCurrent(window_);
  make_current_count = 1;
}

void Context::DetachCurrent() {
  --make_current_count;
  if (make_current_count > 0) return;

  glfwMakeContextCurrent(nullptr);
  g_current = nullptr;
  context_lock_.unlock();
}

bool Context::IsCurrent() const { return g_current == this; }

void Context::RegisterResourceOnCurrent(shared_ptr<IContextResource> resource) {
  assert(g_current != nullptr);
  g_current->resources_.insert(resource);
}

void Context::Resize(int width, int height) {
  ScopedCurrent curr(*this);
  glfwSetWindowSize(window_, width, height);
  width_ = width;
  height_ = height;
}

void Context::Render(const Eigen::Matrix4f &projection,
                     const Eigen::Matrix4f &camera,
                     shared_ptr<GLFramebuffer> framebuffer,
                     shared_ptr<Scene> scene, int width, int height) {
  ScopedCurrent curr(*this);

  if (width < 1) {
    width = width_;
    height = height_;
  }

  framebuffer->SetSize(width, height);
  framebuffer->Bind(true);

  glViewport(0, 0, width, height);
  GLCheckError();

  RenderFrameImpl(scene, projection, camera);
  glfwSwapBuffers(window_);
  glFlush();
  framebuffer->Bind(false);
}

void Context::RenderFrameImpl(shared_ptr<Scene> scene,
                              const Eigen::Matrix4f &projection,
                              const Eigen::Matrix4f &camera) {
  float ratio;
  int width, height;

  glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
  GLCheckError();

  glEnable(GL_DEPTH_TEST);
  GLCheckError();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GLCheckError();

  scene->Draw(projection, camera);
}

shared_ptr<Viewer> Context::CreateViewer(shared_ptr<Scene> scene,
                                         CameraManipulator cam_manip) {
  shared_ptr<ICameraManipulator> cam_manip_obj;

  switch (cam_manip) {
    case CameraManipulator::kTrackBall:
      cam_manip_obj = make_shared<TrackballCameraManipulator>();
      break;
    case CameraManipulator::kWASD:
      cam_manip_obj = make_shared<WASDCameraManipulator>();
      break;
  }

  SharedWindow window(window_, &context_lock_);
  if (viewer_count_ > 0) {
    window = SharedWindow(
        glfwCreateWindow(width_, height_, "Viewer", nullptr, window_), nullptr);
  }
  ++viewer_count_;

  auto viewer =
      shared_ptr<Viewer>(new Viewer(window, this, scene, cam_manip_obj));

  return viewer;
}
}  // namespace tenviz
