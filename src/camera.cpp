#include "camera.hpp"

#include <math.h>

#include "math.hpp"

#include <iostream>

using namespace std;
using namespace Eigen;

namespace tenviz {

namespace camera {

float GetMinBoundingDistance(float radius, float fov_y) {
  using std::cos;
  using std::sin;

  fov_y = fov_y / 2.0f;
  const float alpha = math::DegToRad(fov_y);
  const float theta = math::DegToRad(90 - fov_y);

  const float distance =
      cos(alpha) * ((sin(theta) * radius) / sin(alpha)) + cos(theta) * radius;
  return distance;
}

void GetSceneFrustumFitTransform(const BSphere &scene_sphere, float fov_y,
                                 float &near, float &distance) {
  distance = GetMinBoundingDistance(scene_sphere.get_radius(), fov_y);
  near = distance - scene_sphere.get_radius();
}

}  // namespace camera

Camera::Camera(const Vector3f &pos, const Vector3f &view, const Vector3f &up)
    : eye_(pos), view_(view), up_(up), world_up_(0.0f, 1.0f, 0.0f) {}

Camera Camera::FromSphereCoordinates(float elevation, float azimuth,
                                     const Eigen::Vector3f &center,
                                     float distance) {
  const float theta = math::DegToRad(elevation);
  const float phi = math::DegToRad(azimuth) + math::kPI * 1.5f;

  Eigen::Vector3f pos(cos(phi) * distance * cos(theta), sin(theta) * distance,
                      -sin(phi) * distance * cos(theta));
  pos += center;

  Eigen::Vector3f view(center - pos);
  view.normalize();

  Eigen::Vector3f right(view.cross(Eigen::Vector3f(0, 1, 0)));
  right.normalize();
  Eigen::Vector3f up(right.cross(view));
  up.normalize();

  return Camera(pos, view, up);
}

Camera Camera::FromViewMatrix(const Eigen::Matrix4f &matrix) {
  const Eigen::Vector3f right(matrix(0, 0), matrix(0, 1), matrix(0, 2));

  const Eigen::Vector3f up(matrix(1, 0), matrix(1, 1), matrix(1, 2));
  const Eigen::Vector3f view(-matrix(2, 0), -matrix(2, 1), -matrix(2, 2));

  const Eigen::Vector3f inv_eye(matrix(0, 3), matrix(1, 3), matrix(2, 3));

  const Eigen::Vector3f eye(
      -inv_eye.dot(Eigen::Vector3f(matrix(0, 0), matrix(1, 0), matrix(2, 0))),
      -inv_eye.dot(Eigen::Vector3f(matrix(0, 1), matrix(1, 1), matrix(2, 1))),
      -inv_eye.dot(Eigen::Vector3f(matrix(0, 2), matrix(1, 2), matrix(2, 2))));

  return Camera(eye, view, up);
}

void Camera::RotateRight(float angle) {
  const Vector3f right(GetRight());
  view_ = AngleAxisf(math::RadToDeg(angle), right) * view_;
  view_.normalize();

  Vector3f right2 = view_.cross(world_up_);

  up_ = right2.cross(view_);
  up_.normalize();
}

void Camera::RotateUp(float angle) {
  view_ = AngleAxisf(math::RadToDeg(angle), up_) * view_;
  view_.normalize();
}

void Camera::RotateView(float angle) {
  up_ = AngleAxisf(math::RadToDeg(angle), view_) * up_;
  up_.normalize();
}

void Camera::RotateAxis(const Eigen::Vector3f &axis, float angle) {
  AngleAxisf aaxis(math::DegToRad(angle), axis);
  eye_ = aaxis * eye_;

  view_ = aaxis * view_;
  view_.normalize();

  up_ = aaxis * up_;
  up_.normalize();
}

void Camera::RotateQuat(const Eigen::Quaternionf &quat) {
  eye_ = quat * eye_;

  view_ = quat * view_;
  view_.normalize();

  up_ = quat * up_;
  up_.normalize();
}

void Camera::Forward(float ac) { eye_ += view_ * ac; }

void Camera::Strife(float ac) {
  const Vector3f right_move = GetRight() * ac;
  eye_ += right_move;
}

void Camera::Move(const Eigen::Vector3f &move) { eye_ += move; }

Matrix4f Camera::GetMatrix() const {
  Eigen::Matrix4f rot(Eigen::Matrix4f::Identity());
  auto right = GetRight();

  const Eigen::Vector3f up = right.cross(view_).normalized();
  const Eigen::Vector3f view = up.cross(right).normalized();
  right = view.cross(up).normalized();
  rot(0, 0) = right[0];
  rot(0, 1) = right[1];
  rot(0, 2) = right[2];
  rot(1, 0) = up_[0];
  rot(1, 1) = up_[1];
  rot(1, 2) = up_[2];
  rot(2, 0) = -view_[0];
  rot(2, 1) = -view_[1];
  rot(2, 2) = -view_[2];

  Eigen::Matrix4f trans(Eigen::Matrix4f::Identity());

  trans(0, 3) = -eye_[0];
  trans(1, 3) = -eye_[1];
  trans(2, 3) = -eye_[2];

  Eigen::Matrix4f view_mtx = rot * trans;
  return view_mtx;
}
}  // namespace tenviz
