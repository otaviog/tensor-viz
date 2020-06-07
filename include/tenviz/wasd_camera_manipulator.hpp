#pragma once

#include <string>

#include "camera.hpp"
#include "camera_manipulator.hpp"
#include "eigen_common.hpp"
#include "projection.hpp"

namespace tenviz {

class WASDCameraManipulator : public ICameraManipulator {
 public:
  WASDCameraManipulator();

  void ResetView(const Bounds &bounds) override;

  Eigen::Matrix4f GetViewMatrix() override { return camera_.GetMatrix(); }

  void SetViewMatrix(const Eigen::Matrix4f &matrix) override;

  Eigen::Matrix4f GetProjectionMatrix(int width, int height,
                                      const Bounds &bounds) override;

  void KeyState(const std::map<int, bool> &keymap, double time,
                const Bounds &bounds) override;

  void KeyPressed(GLFWwindow *window, int glfw_key, double elapsed_time,
                  const Bounds &bounds) override { }

  void CursorMoved(GLFWwindow *window, double xpos, double ypos) override;

  void CursorButtonPressed(GLFWwindow *window, int glfw_button, int glfw_action,
                           int glfw_mods) override;

  void CursorScrollMoved(GLFWwindow *window, double xoffset,
                         double yoffset) override;

  void set_velocity(float vel) { velocity_ = vel; }

  float get_velocity() const { return velocity_; }

 private:
  Camera camera_;

  float velocity_;

  Eigen::Vector2f mouse_last_pos_, rot_sensitivity_;
};
}  // namespace tenviz
