#include "projection.hpp"
#include "math.hpp"

#include "gl_common.hpp"

namespace tenviz {

Projection Projection::Perspective(float fov_y, float aspect, float near,
                                   float far) {
  float top = std::tan(math::DegToRad(fov_y / 2.0f)) * near;
  float right = top * aspect;
  return Projection(-right, right, -top, top, near, far);
}

Projection::Projection(float _left, float _right, float _bot, float _top,
                       float _near, float _far) {
  left_ = _left;
  right_ = _right;
  bottom_ = _bot;
  top_ = _top;
  near_ = _near;
  far_ = _far;

  far_left_ = (left_ / near_) * far_;
  far_right_ = (right_ / near_) * far_;
  far_top_ = (top_ / near_) * far_;
  far_bottom_ = (bottom_ / near_) * far_;

  fov_ = math::RadToDeg(std::atan(right_ / near_)) * 2;
  aspect_ = (right_ + std::abs(left_)) / (top_ + std::abs(bottom_));
}

Eigen::Matrix4f Projection::GetMatrix() const {
  Eigen::Matrix4f proj_mat = Eigen::Matrix4f::Zero();

  proj_mat(0, 0) = 2.0 * near_ / (right_ - left_);
  proj_mat(0, 1) = 0.0;
  proj_mat(0, 2) = (right_ + left_) / (right_ - left_);
  proj_mat(0, 3) = 0.0;

  proj_mat(1, 0) = 0.0;
  proj_mat(1, 1) = 2.0 * near_ / (top_ - bottom_);
  proj_mat(1, 2) = (top_ + bottom_) / (top_ - bottom_);
  proj_mat(1, 3) = 0;

  proj_mat(2, 0) = 0.0;
  proj_mat(2, 1) = 0.0;
  proj_mat(2, 2) = -(far_ + near_) / (far_ - near_);
  proj_mat(2, 3) = -(2.0 * far_ * near_) / (far_ - near_);

  proj_mat(3, 0) = 0.0;
  proj_mat(3, 1) = 0.0;
  proj_mat(3, 2) = -1.0;
  proj_mat(3, 3) = 0.0;

  return proj_mat;
}
}  // namespace tenviz
