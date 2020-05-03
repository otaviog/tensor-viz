#pragma once

#include <sstream>

#include <torch/csrc/utils/pybind.h>
#include <torch/torch.h>
#include <sophus/interpolate.hpp>
#include <sophus/se3.hpp>

#include "eigen_common.hpp"

namespace tenviz {

class Pose;

class SE3 : public Sophus::SE3d {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  SE3() {}

  SE3(const QuaternionType &quat, const Point &point)
      : Sophus::SE3d(quat, point) {}

  SE3(const Sophus::SE3d &copy) : Sophus::SE3d(copy){};

  static SE3 FromMatrix(const torch::Tensor &matrix) {
    return SE3(Sophus::SE3d(from_tensorm4<double>(matrix)));
  }

  static SE3 FromRotationMatrixTranslation(const torch::Tensor &rot_matrix,
                                           const torch::Tensor &trans) {
    return SE3(Sophus::SE3d(from_tensorm3<double>(rot_matrix),
                            from_tensorv3<double>(trans)));
  }

  static SE3 FromQuaternionTranslation(const torch::Tensor &quat,
                                       const torch::Tensor &trans) {
    const auto quat_acc = quat.accessor<double, 1>();
    return SE3(Sophus::SE3d(
        QuaternionType(quat_acc[0], quat_acc[1], quat_acc[2], quat_acc[3]),
        from_tensorv3<double>(trans)));
  }

  static SE3 FromPose(const Pose &pose);

  static void RegisterPybind(pybind11::module &m);

  torch::Tensor ToMatrix() {
    torch::Tensor out = torch::zeros({4, 4}, torch::TensorOptions(torch::kDouble));
    Eigen::Matrix<Scalar, 3, 4> mat = matrix3x4();
    AT_DISPATCH_ALL_TYPES(out.scalar_type(), "to_matrix", ([&] {
                            auto acc = out.accessor<scalar_t, 2>();
                            acc[3][0] = 0;
                            acc[3][1] = 0;
                            acc[3][2] = 0;
                            acc[3][3] = 1;
                            for (int i = 0; i < 3; ++i) {
                              for (int j = 0; j < 4; ++j) {
                                acc[i][j] = mat(i, j);
                              }
                            }
                          }));
    return out;
  }

  torch::Tensor torch_params() {
    torch::Tensor out = torch::empty({num_parameters});
    auto p = params();
    AT_DISPATCH_ALL_TYPES(out.scalar_type(), "get_params", ([&] {
                            auto acc = out.accessor<scalar_t, 1>();
                            for (int i = 0; i < num_parameters; ++i) {
                              acc[i] = p[i];
                            }
                          }));
    return out;
  }

  static SE3 torch_exp(const torch::Tensor tangent) {
    Tangent t;
    AT_DISPATCH_ALL_TYPES(tangent.scalar_type(), "exp", ([&] {
                            const auto acc = tangent.accessor<scalar_t, 1>();
                            t << acc[0], acc[1], acc[2], acc[3], acc[4], acc[5];
                          }));

    return SE3(Sophus::SE3<double>::exp(t));
  }

  torch::Tensor torch_log() {
    Tangent t = Sophus::SE3<double>::log();
    torch::Tensor out_tangent = torch::empty({6});
    AT_DISPATCH_ALL_TYPES(out_tangent.scalar_type(), "exp", ([&] {
                            auto acc = out_tangent.accessor<scalar_t, 1>();
                            for (int i = 0; i < 6; ++i) acc[i] = t[i];
                          }));
    return out_tangent;
  }

  SE3 Interpolate(const SE3 &other, double t) {
    return SE3(
        Sophus::interpolate<Sophus::SE3<double>, double>(*this, other, t));
  }

  torch::Tensor torch_quaternion() const {
    const auto quat = unit_quaternion();
    torch::Tensor out_quat = torch::empty({4}, torch::kDouble);
    auto acc = out_quat.accessor<double, 1>();
    acc[0] = quat.w();
    acc[1] = quat.x();
    acc[2] = quat.y();
    acc[3] = quat.z();

    return out_quat;
  }

  torch::Tensor torch_translation() {
    torch::Tensor tensor = torch::empty({3}, torch::kDouble);
    auto acc = tensor.accessor<double, 1>();

    acc[0] = translation()[0];
    acc[1] = translation()[1];
    acc[2] = translation()[2];

    return tensor;
  }

  std::string __str__() const {
    const auto xyz = translation();
    std::stringstream str;
    str << "X-rot: " << angleX() << " Y-rot: " << angleY()
        << " Z-rot: " << angleZ() << " X: " << xyz[0] << " Y: " << xyz[1]
        << " Z: " << xyz[2];
    return str.str();
  }
};

}  // namespace tenviz
