#include "wasd_camera_manipulator.hpp"

#include <iostream>
#include <sstream>
#include "math.hpp"

using namespace std;

namespace tenviz {
using namespace Eigen;

WASDCameraManipulator::WASDCameraManipulator() : rot_sensitivity_(0.1, 0.1) {
  velocity_ = 0.025f;
}

void WASDCameraManipulator::SetViewMatrix(const Eigen::Matrix4f &matrix) {
  camera_ = Camera::FromViewMatrix(matrix);
}

void WASDCameraManipulator::ResetView(const Bounds &bounds) {
  float near, distance;
  const BSphere &sphere = bounds.get_sphere();
  camera::GetSceneFrustumFitTransform(sphere, 45.0f, near, distance);
  camera_.set_view(Eigen::Vector3f(0, 0, -1));
  camera_.set_eye(Eigen::Vector3f(0, 0, distance));
}

Eigen::Matrix4f WASDCameraManipulator::GetProjectionMatrix(
    int screen_width, int screen_height, const Bounds &bounds) {
  const float aspect = screen_width < screen_height
                           ? float(screen_height) / float(screen_width)
                           : float(screen_width) / float(screen_height);

  const BSphere &sphere = bounds.get_sphere();
  float near, distance;
  camera::GetSceneFrustumFitTransform(sphere, 45.0f, near, distance);

  distance = (camera_.get_eye() - sphere.get_center()).norm() +
             sphere.get_radius() * 2;

  Projection proj = Projection::Perspective(45.0f, aspect, near * 0.001, 2000);
  return proj.GetMatrix();
}

void WASDCameraManipulator::KeyState(const map<int, bool> &keymap,
                                     double elap_secs, const Bounds &bounds) {
  const float move_incr =
      velocity_ * bounds.get_sphere().get_radius() * 2 * elap_secs;
  auto is_key_pressed = [&](int key) {
    const auto iter = keymap.find(key);
    if (iter != keymap.end()) {
      return iter->second;
    }
    return false;
  };

  if (is_key_pressed(GLFW_KEY_W)) {
    camera_.Forward(move_incr);
  }

  if (is_key_pressed(GLFW_KEY_A)) {
    camera_.Strife(-move_incr);
  }

  if (is_key_pressed(GLFW_KEY_S)) {
    camera_.Forward(-move_incr);
  }

  if (is_key_pressed(GLFW_KEY_D)) {
    camera_.Strife(move_incr);
  }
}

void WASDCameraManipulator::CursorMoved(GLFWwindow *window, double xpos,
                                        double ypos) {
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS) {
    return;
  }

  int width, height;
  glfwGetWindowSize(window, &width, &height);

  const Vector2f curr_pos(xpos, ypos);
  Vector2f diff = (mouse_last_pos_ - curr_pos);
  diff[0] = diff[0] / float(width) * rot_sensitivity_[0];
  diff[1] = diff[1] / float(height) * rot_sensitivity_[1];

  camera_.RotateUp(diff[0]);
  camera_.RotateRight(diff[1]);

  mouse_last_pos_ = curr_pos;
}

void WASDCameraManipulator::CursorButtonPressed(GLFWwindow *window, int button,
                                                int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mouse_last_pos_ = Vector2f(xpos, ypos);
  }
}

void WASDCameraManipulator::CursorScrollMoved(GLFWwindow *window,
                                              double xoffset, double yoffset) {}
}  // namespace tenviz
