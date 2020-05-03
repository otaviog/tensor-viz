#pragma once
#include "gl_common.hpp"

#include <string>

#include <torch/csrc/utils/pybind.h>
#include <torch/torch.h>

#include "camera_manipulator.hpp"
#include "context.hpp"
#include "projection.hpp"
#include "time_measurer.hpp"

namespace tenviz {

class SharedWindow {
 public:
  SharedWindow(GLFWwindow *window, std::mutex *lock);

  void MakeCurrent();

  void DetachCurrent();

  void Release();

  GLFWwindow *handle;

  bool is_shared() const { return lock_ != nullptr; }

 private:
  std::mutex *lock_;
};

class Viewer {
 public:
  Viewer(SharedWindow shared_window, Context *orig_context,
         std::shared_ptr<Scene> scene,
         std::shared_ptr<ICameraManipulator> cam_manip);

  virtual ~Viewer() { Release(); }

  static void RegisterPybind(pybind11::module &m);

  void Release();

  void ResetView();

  int WaitKey(int swap_interval);

  bool Draw(int swap_interval = 2);

  void SetTitle(const std::string &title);

  const std::string &get_title() const { return title_; }

  void SetProjection(const Projection &proj);

  void py_set_projection(float left, float right, float bottom, float top,
                         float near, float far) {
    SetProjection(Projection(left, right, bottom, top, near, far));
  }

  Eigen::Matrix4f GetProjectionMatrix() const;

  void SetCameraManipulator(std::shared_ptr<ICameraManipulator> manip) {
    camera_manip_ = manip;
  }

  void SetViewMatrix(const Eigen::Matrix4f &view) {
    camera_manip_->SetViewMatrix(view);
    first_view_ = false;
  }

  Eigen::Matrix4f GetViewMatrix() const {
    return camera_manip_->GetViewMatrix();
  }

  std::shared_ptr<Scene> get_scene() { return scene_; }

  int get_width() const { return width_; }

  int get_height() const { return height_; }

 private:
  void UpdateSize(int width, int height);

  SharedWindow window_;
  Context *orig_context_;

  std::shared_ptr<Scene> scene_;

  int width_, height_;
  std::string title_;
  bool first_view_;
  int last_key_;

  std::shared_ptr<ICameraManipulator> camera_manip_;
  std::pair<bool, Projection> user_projection_;

  TimeMeasurer frame_tick_;

  friend void KeyCallback(GLFWwindow *, int, int, int, int);
  friend void CursorPositionCallback(GLFWwindow *, double, double);
  friend void MouseButtonCallback(GLFWwindow *, int, int, int);
  friend void ScrollCallback(GLFWwindow *, double, double);
  friend void WindowSizeCallback(GLFWwindow *, int, int);
};
}  // namespace tenviz
