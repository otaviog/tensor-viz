#include "trackball_camera_manipulator.hpp"

#include "camera.hpp"
#include "math.hpp"

#include <iostream>

using namespace std;

using namespace Eigen;

namespace tenviz {
namespace {

static float DEFAULT_FOV = 45.0f;

Eigen::Vector3f GetArcBall(const Eigen::Vector2i& mpos, float width,
                           float height) {
  const float x = mpos[0];
  const float y = mpos[1];
  Vector3f pos((x / width) * 2.0f - 1.0f, 1.0f - (y / height) * 2.0f, 0.0f);

  const float radius = 0.5f;
  const float sq_radius = radius * radius;
  const float sq_norm = pos[0] * pos[0] + pos[1] * pos[1];

  if (sq_norm <= sq_radius / 2.0f) {
    pos[2] = std::sqrt(sq_radius - sq_norm);
  } else {
    pos[2] = (sq_radius * 0.5f) / std::sqrt(sq_norm);
  }

  return pos.normalized();
}
}  // namespace

TrackballCameraManipulator::TrackballCameraManipulator()
    : rot_(Eigen::Quaternionf(
          Eigen::AngleAxisf(0.0, Eigen::Vector3f(0.0f, 1.0f, 0.0f)))) {
  ResetView(Bounds::UnitBounds());
}

void TrackballCameraManipulator::ResetView(const Bounds& bounds) {
  float distance, near;
  camera::GetSceneFrustumFitTransform(bounds.get_sphere(), DEFAULT_FOV, near,
                                      distance);
  camera_.set_eye(Eigen::Vector3f(0.0, 0.0, distance));
  ref_camera_ = camera_;

  m_bounds = bounds;
}

Eigen::Matrix4f TrackballCameraManipulator::GetProjectionMatrix(
    int screen_width, int screen_height, const Bounds& bounds) {
  const float aspect = screen_width < screen_height
                           ? float(screen_height) / float(screen_width)
                           : float(screen_width) / float(screen_height);
  float distance, near;
  camera::GetSceneFrustumFitTransform(bounds.get_sphere(), DEFAULT_FOV, near,
                                      distance);

  Projection proj = Projection::Perspective(DEFAULT_FOV, aspect, near * 0.001,
                                            distance * 2.0);

  return proj.GetMatrix();
}

void TrackballCameraManipulator::KeyPressed(GLFWwindow* window, int key) {
  float incr = 5.0;
  switch (key) {
    case GLFW_KEY_W:
      rot_ *= Quaternionf(AngleAxisf(incr, Eigen::Vector3f(1.0f, 0.0f, 0.0f)));
      break;
    case GLFW_KEY_S:
      rot_ *= Quaternionf(AngleAxisf(-incr, Eigen::Vector3f(1.0f, 0.0f, 0.0f)));
      break;
    case GLFW_KEY_A:
      rot_ *= Quaternionf(AngleAxisf(-incr, Eigen::Vector3f(0.0f, 1.0f, 0.0f)));
      break;
    case GLFW_KEY_D:
      rot_ *= Quaternionf(AngleAxisf(incr, Eigen::Vector3f(0.0f, 1.0f, 0.0f)));
      break;
    default:
      break;
  }

  camera_ = ref_camera_;
  camera_.RotateQuat(rot_);
}

void TrackballCameraManipulator::CursorButtonPressed(GLFWwindow* window,
                                                     int button, int action,
                                                     int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mouse_last_pos_ = Vector2i(xpos, ypos);
  }
}

void TrackballCameraManipulator::UpdateRotation(GLFWwindow* window, double xpos,
                                                double ypos) {
  int width, height;
  glfwGetWindowSize(window, &width, &height);

  Eigen::Vector2i curr_pos(xpos, ypos);
  if (curr_pos == mouse_last_pos_) return;

  Eigen::Vector3f curr_ab(GetArcBall(curr_pos, width, height));
  Eigen::Vector3f last_ab(GetArcBall(mouse_last_pos_, width, height));

  const float angle = math::RadToDeg(std::acos(curr_ab.dot(last_ab)));
  const Eigen::Vector3f cam_axis = curr_ab.cross(last_ab).normalized();

  rot_ *=
      Eigen::Quaternionf(Eigen::AngleAxisf(math::DegToRad(angle), cam_axis));
  camera_ = ref_camera_;
  camera_.RotateQuat(rot_);
  mouse_last_pos_ = curr_pos;
}

void TrackballCameraManipulator::UpdateTranslation(GLFWwindow* window,
                                                   double xpos, double ypos) {
  const float move_sensitivity = 0.01;

  int width, height;
  glfwGetWindowSize(window, &width, &height);

  const Eigen::Vector2i curr_pos(xpos, ypos);
  const Eigen::Vector2i diff(mouse_last_pos_ - curr_pos);

  Eigen::Vector3f move(diff[0], -diff[1], 0.0f);
  move.normalize();
  move *= move_sensitivity * m_bounds.get_sphere().get_radius();

  ref_camera_.Move(move);
  camera_ = ref_camera_;
  camera_.RotateQuat(rot_);

  mouse_last_pos_ = curr_pos;
}

void TrackballCameraManipulator::CursorMoved(GLFWwindow* window, double xpos,
                                             double ypos) {
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    UpdateRotation(window, xpos, ypos);
  }

  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
    UpdateTranslation(window, xpos, ypos);
  }
}

void TrackballCameraManipulator::CursorScrollMoved(GLFWwindow* window, double,
                                                   double yoffset) {
  const float move_sensitivity = 0.1;
  ref_camera_.Forward(yoffset * move_sensitivity *
                      m_bounds.get_sphere().get_radius());
  camera_ = ref_camera_;
  camera_.RotateQuat(rot_);
}

}  // namespace tenviz
