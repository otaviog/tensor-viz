#pragma once

#include "camera.hpp"
#include "camera_manipulator.hpp"
#include "error.hpp"
#include "projection.hpp"

namespace tenviz {

class TrackballCameraManipulator : public ICameraManipulator {
 public:
  TrackballCameraManipulator();

  void ResetView(const Bounds& bounds) override;

  Eigen::Matrix4f GetViewMatrix() override { return camera_.GetMatrix(); }

  void SetViewMatrix(const Eigen::Matrix4f& matrix) override {
    throw Error("Operation not implemented");
  }

  Eigen::Matrix4f GetProjectionMatrix(int screen_width, int screen_height,
                                      const Bounds& bounds) override;

  void KeyState(GLFWwindow* window, float fps, const Bounds& bounds) override {}

  void KeyPressed(GLFWwindow* window, int glfw_key) override;

  void CursorMoved(GLFWwindow* window, double xpos, double ypos) override;

  void CursorButtonPressed(GLFWwindow* window, int glfw_button, int glfw_action,
                           int glfw_mods) override;

  void CursorScrollMoved(GLFWwindow* window, double xoffset,
                         double yoffset) override;

 private:
  void UpdateRotation(GLFWwindow* window, double xpos, double ypos);

  void UpdateTranslation(GLFWwindow* window, double xpos, double ypos);

  Camera camera_, ref_camera_;
  Eigen::Quaternionf rot_;
  Eigen::Vector2i mouse_last_pos_;

  Bounds m_bounds;
};
}  // namespace tenviz
