#pragma once

#include "eigen_common.hpp"

#include <torch/csrc/utils/pybind.h>
#include <torch/torch.h>

#include "se3.hpp"

namespace tenviz {

class SE3;

/**
 * Pose, or rigid transformation, representation using a 3D xyz and a quaternon.
 */
class Pose {
 public:
  typedef double Scalar;

  typedef Eigen::Matrix<Scalar, 3, 1> Point3;
  typedef Eigen::Quaternion<Scalar> Quaternion;

  Pose(const Quaternion &quat = Quaternion(1, 0, 0, 0),
       const Point3 &translation = Point3::Zero())
      : quat_(quat), translation_(translation) {}
  
  Pose(float qw, float qx, float qy, float qz, float tx, float ty, float tz)
      : quat_(qw, qx, qy, qz), translation_(tx, ty, tz) {}

  static Pose FromMatrix(const torch::Tensor &matrix) {
    Quaternion quat(from_tensorm3<double>(matrix));
    Point3 trans;
    AT_DISPATCH_ALL_TYPES(matrix.scalar_type(), "Pose::FromMatrix", ([&] {
                            const auto acc = matrix.accessor<scalar_t, 2>();

                            trans[0] = acc[0][3];
                            trans[1] = acc[1][3];
                            trans[2] = acc[2][3];
                          }));
    return Pose(quat, trans);
  }

  static Pose FromRotationMatrixTranslation(const torch::Tensor &rot_matrix,
                                            const torch::Tensor &translation) {
    const Quaternion quat(from_tensorm3<Scalar>(rot_matrix));
    const Point3 trans = from_tensorv3<Scalar>(translation);

    return Pose(quat, trans);
  }

  static Pose FromSE3(const SE3 &se3) {
    return Pose(se3.unit_quaternion(), se3.translation());
  }

  static void RegisterPybind(pybind11::module &m);

  torch::Tensor ToMatrix() const {
    torch::Tensor matrix = torch::empty({4, 4}, torch::kDouble);
    auto acc = matrix.accessor<double, 2>();

    const auto rot_mtx = quat_.toRotationMatrix();
    acc[0][0] = rot_mtx(0, 0);
    acc[0][1] = rot_mtx(0, 1);
    acc[0][2] = rot_mtx(0, 2);

    acc[1][0] = rot_mtx(1, 0);
    acc[1][1] = rot_mtx(1, 1);
    acc[1][2] = rot_mtx(1, 2);

    acc[2][0] = rot_mtx(2, 0);
    acc[2][1] = rot_mtx(2, 1);
    acc[2][2] = rot_mtx(2, 2);

    acc[0][3] = translation_[0];
    acc[1][3] = translation_[1];
    acc[2][3] = translation_[2];

    acc[3][0] = 0;
    acc[3][1] = 0;
    acc[3][2] = 0;
    acc[3][3] = 1;

    return matrix;
  }

  Pose Inverse() const {
    const auto quat_inv = quat_.inverse();
    const Point3 trans = quat_inv * -translation_;

    return Pose(quat_inv, trans);
  }

  Pose Interpolate(const Pose &other, Scalar t) const {
    const Pose inv = Inverse();
    const Pose h = other * inv;

    const auto se3 = SE3::FromPose(h);
    const auto interp_se3 = SE3().Interpolate(se3, t);

    return Pose::FromSE3(interp_se3) * *this;
  }

  pybind11::object get_euler_angles() const {
    const auto euler = quat_.toRotationMatrix().eulerAngles(0, 1, 2);
    return pybind11::make_tuple(euler[0], euler[1], euler[2]);
  }

  pybind11::object get_quaternion() const{
    return pybind11::make_tuple(quat_.w(), quat_.x(), quat_.y(),
                                quat_.z());
  }

  pybind11::object get_translation() const {
    return pybind11::make_tuple(translation_[0], translation_[1], translation_[2]);
  }
  
  Pose operator*(const Pose &other) const {
    Point3 translation = (quat_ * other.translation());
    translation += translation_;
    Quaternion quat = quat_ * other.quaternion();

    return Pose(quat, translation);
  }
  
  std::string __str__() const {
    std::stringstream str;
    str << "{qw: " << quat_.w() << ", qx: " << quat_.x()
        << ", qy: " << quat_.y() << ", qz: " << quat_.z()
        << ", tx: " << translation_[0] << ", ty: " << translation_[1]
        << ", tz: " << translation_[2] << "}";
    return str.str();
  }

  const Point3 &translation() const { return translation_; }

  const Eigen::Quaternion<Scalar> quaternion() const { return quat_; }

 private:
  Eigen::Quaternion<Scalar> quat_;
  Point3 translation_;
};
}  // namespace tenviz
