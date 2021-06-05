#include "viewer.hpp"

#include <sstream>

#include <pybind11/eigen.h>

#include "camera.hpp"
#include "camera_manipulator.hpp"
#include "gl_common.hpp"
#include "gl_error.hpp"
#include "gl_shader_program.hpp"
#include "scene.hpp"
#include "trackball_camera_manipulator.hpp"

using namespace std;
using namespace Eigen;

namespace tenviz {

namespace {
const int NO_LAST_KEY = std::numeric_limits<int>::min();

void HandleCtrlC(GLFWwindow* window,
                 std::shared_ptr<ICameraManipulator> camera_manip) {
  const Eigen::Matrix4f mtx = camera_manip->GetViewMatrix();
  std::ostringstream buffer;

  // clang-format off
        buffer
            << "[[ " << mtx(0, 0) << " , " << mtx(0, 1) << " , " << mtx(0, 2) << " , " << mtx(0, 3) << " ]," << std::endl
            << "[ " << mtx(1, 0) << " , " << mtx(1, 1) << " , " << mtx(1, 2) << " , " << mtx(1, 3) << " ]," << std::endl
            << "[ " << mtx(2, 0) << " , " << mtx(2, 1) << " , " << mtx(2, 2) << " , " << mtx(2, 3) << " ]," << std::endl
            << "[ " << mtx(3, 0) << " , " << mtx(3, 1) << " , " << mtx(3, 2) << " , " << mtx(3, 3) << " ]]" << std::endl;
  // clang-format on
  glfwSetClipboardString(window, buffer.str().c_str());
}

void HandleCtrlV(GLFWwindow* window,
                 std::shared_ptr<ICameraManipulator> camera_manip) {
  const char* c_state = glfwGetClipboardString(window);
  if (c_state) {
    std::istringstream buffer_stream(c_state);
    Eigen::Matrix4f mtx;

    for (int i = 0; i < 4; ++i) {
      std::string line;
      std::getline(buffer_stream, line);

      std::istringstream line_stream(line);
      line_stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
      for (int j = 0; j < 4; ++j) {
        std::string temp;
        line_stream >> mtx(i, j);
        line_stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
        line_stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
      }
    }

    camera_manip->SetViewMatrix(mtx);
  }
}
}  // namespace

void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mods) {
  Viewer* self = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
  if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
    glfwSetWindowShouldClose(window, 1);
    return;
  }

  if (action == GLFW_PRESS) {
    self->pressed_key_map_[key] = true;
    if (mods == 0) {
      self->camera_manip_->KeyPressed(window, key, self->elapsed_,
                                      self->scene_->GetBounds());
      self->last_key_ = key;
    }
  } else if (action == GLFW_RELEASE) {
    self->pressed_key_map_[key] = false;

    if (mods == GLFW_MOD_CONTROL) {
      if (key == GLFW_KEY_C) {
        HandleCtrlC(window, self->camera_manip_);
      } else if (key == GLFW_KEY_V) {
        HandleCtrlV(window, self->camera_manip_);
      }
    }
  }
}

void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
  Viewer* self = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
  self->camera_manip_->CursorMoved(window, xpos, ypos);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  Viewer* self = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
  self->camera_manip_->CursorButtonPressed(window, button, action, mods);
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  Viewer* self = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
  self->camera_manip_->CursorScrollMoved(window, xoffset, yoffset);
}

void WindowSizeCallback(GLFWwindow* window, int width, int height) {
  Viewer* self = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
  self->UpdateSize(width, height);
}

SharedWindow::SharedWindow(GLFWwindow* window, std::mutex* lock) {
  handle = window;
  lock_ = lock;
}

void SharedWindow::Release() {
  if (handle != nullptr && !is_shared()) {
    MakeCurrent();
    glfwSetWindowShouldClose(handle, 1);
    glfwDestroyWindow(handle);
    DetachCurrent();
    handle = nullptr;
  }
}

void SharedWindow::MakeCurrent() {
  if (lock_ != nullptr) {
    lock_->lock();
  }
  glfwMakeContextCurrent(handle);
}

void SharedWindow::DetachCurrent() {
  glfwMakeContextCurrent(nullptr);

  if (lock_ != nullptr) {
    lock_->unlock();
  }
}

Viewer::Viewer(SharedWindow shared_window, Context* orig_context,
               std::shared_ptr<Scene> scene,
               std::shared_ptr<ICameraManipulator> cam_manip)
    : window_(shared_window),
      orig_context_(orig_context),
      scene_(scene),
      camera_manip_(cam_manip),
      user_projection_(make_pair(
          false, Projection::Perspective(45.0f, 1.0f, 1.0f, 1000.0f))) {
  last_key_ = NO_LAST_KEY;
  first_view_ = true;

  glfwSetWindowUserPointer(window_.handle, this);
  glfwSetKeyCallback(window_.handle, KeyCallback);
  glfwSetCursorPosCallback(window_.handle, CursorPositionCallback);

  glfwSetMouseButtonCallback(window_.handle, MouseButtonCallback);
  glfwSetScrollCallback(window_.handle, ScrollCallback);
  glfwSetWindowSizeCallback(window_.handle, WindowSizeCallback);

  glfwGetWindowSize(window_.handle, &width_, &height_);
}

void Viewer::Release() { window_.Release(); }

void Viewer::UpdateSize(int width, int height) {
  width_ = width;
  height_ = height;
}

void Viewer::SetTitle(const std::string& title) {
  glfwSetWindowTitle(window_.handle, title.c_str());
  title_ = title;
}

void Viewer::ResetView() { camera_manip_->ResetView(scene_->GetBounds()); }

Eigen::Matrix4f Viewer::GetProjectionMatrix() const {
  if (user_projection_.first) {
    return user_projection_.second.GetMatrix();
  } else {
    return camera_manip_->GetProjectionMatrix(width_, height_,
                                              scene_->GetBounds());
  }
}

void Viewer::SetProjection(const Projection& proj) {
  user_projection_ = make_pair(true, proj);
}

class ScopedContext {
 public:
  ScopedContext(SharedWindow wnd) : wnd_(wnd) { wnd_.MakeCurrent(); }
  ~ScopedContext() { wnd_.DetachCurrent(); }

 private:
  SharedWindow wnd_;
};

bool Viewer::Draw(int swap_interval) {
  ScopedContext curr(window_);
  // orig_context_->MakeCurrent();
  glfwSwapInterval(swap_interval);

  if (!glfwGetWindowAttrib(window_.handle, GLFW_VISIBLE)) {
    glfwShowWindow(window_.handle);
  }

  glViewport(0, 0, width_, height_);
  GLCheckError();

  const Eigen::Matrix4f view_mtx = camera_manip_->GetViewMatrix();

  Eigen::Matrix4f proj_mtx = GetProjectionMatrix();

  orig_context_->RenderFrameImpl(scene_, proj_mtx, view_mtx);
  glfwSwapBuffers(window_.handle);

  glfwPollEvents();

  elapsed_ = frame_tick_.Tick();
  camera_manip_->KeyState(pressed_key_map_, elapsed_, scene_->GetBounds());

  return !glfwWindowShouldClose(window_.handle);
}

int Viewer::WaitKey(int swap_interval) {
  if (first_view_) {
    first_view_ = false;
    ResetView();
  }

  TimeMeasurer time_meas;
  while (true) {
    if (!Draw(swap_interval)) {
      break;
    }

    if (last_key_ != NO_LAST_KEY) {
      int r_key = last_key_;
      last_key_ = NO_LAST_KEY;
      return r_key;
    }

    if (time_meas.GetElapsed() > swap_interval) {
      return 0;
    }

    if (swap_interval == 0) {
      return 0;
    }
  }
  return -1;
}

void Viewer::RegisterPybind(pybind11::module& m) {
  namespace py = pybind11;
  py::class_<Viewer, shared_ptr<Viewer>>(m, "Viewer")
      .def("reset_view", &Viewer::ResetView)
      .def("draw", &Viewer::Draw, py::arg("swap_interval") = int(2))
      .def("wait_key", &Viewer::WaitKey)
      .def("release", &Viewer::Release)
      .def("get_scene", &Viewer::get_scene)
      .def_property("context", &Viewer::get_context, nullptr)
      .def_property("title", &Viewer::get_title, &Viewer::SetTitle)
      .def("set_projection", &Viewer::py_set_projection)
      .def_property("projection_matrix", &Viewer::GetProjectionMatrix, nullptr)
      .def_property("view_matrix", &Viewer::GetViewMatrix,
                    &Viewer::SetViewMatrix)
      .def_property("width", &Viewer::get_width, nullptr)
      .def_property("height", &Viewer::get_height, nullptr);
}

}  // namespace tenviz
