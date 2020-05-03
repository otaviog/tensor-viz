#pragma once

#include <sstream>

#include <torch/csrc/utils/pybind.h>
#include <torch/torch.h>
#include <sophus/interpolate.hpp>
#include <sophus/se3.hpp>

#include "eigen_common.hpp"

namespace tenviz {

class Pose;

class SO3 : public Sophus::SO3d {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  SO3() {}

  SO3(const QuaternionType &quat) : Sophus::SO3d(quat) {}

  SO3(const Sophus::SO3d &copy) : Sophus::SO3d(copy){};

  static SO3 FromMatrix(const torch::Tensor &matrix) {
    return SO3(Sophus::SO3d(from_tensorm3<double>(matrix)));
  }

  static SO3 FromQuaternion(const torch::Tensor &quat) {
    const auto quat_acc = quat.accessor<double, 1>();
    return SO3(Sophus::SO3d(
        QuaternionType(quat_acc[0], quat_acc[1], quat_acc[2], quat_acc[3])));
  }

  static void RegisterPybind(pybind11::module &m);

  torch::Tensor ToMatrix() {
    torch::Tensor out = torch::zeros({4, 4});
    Eigen::Matrix<Scalar, 3, 3> mat = matrix();
    AT_DISPATCH_ALL_TYPES(out.scalar_type(), "to_matrix", ([&] {
                            auto acc = out.accessor<scalar_t, 2>();
                            for (int i = 0; i < 3; ++i) {
                              for (int j = 0; j < 3; ++j) {
                                acc[i][j] = mat(i, j);
                              }
                            }
                            acc[3][3] = 1;
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

  static SO3 torch_exp(const torch::Tensor tangent) {
    Tangent t;
    AT_DISPATCH_ALL_TYPES(tangent.scalar_type(), "exp", ([&] {
                            const auto acc = tangent.accessor<scalar_t, 1>();
                            t << acc[0], acc[1], acc[2];
                          }));

    return SO3(Sophus::SO3<double>::exp(t));
  }

  torch::Tensor torch_log() {
    Tangent t = Sophus::SO3<double>::log();
    torch::Tensor out_tangent = torch::empty({3});
    AT_DISPATCH_ALL_TYPES(out_tangent.scalar_type(), "exp", ([&] {
                            auto acc = out_tangent.accessor<scalar_t, 1>();
                            for (int i = 0; i < 3; ++i) acc[i] = t[i];
                          }));
    return out_tangent;
  }

  SO3 Interpolate(const SO3 &other, double t) {
    return SO3(
        Sophus::interpolate<Sophus::SO3<double>, double>(*this, other, t));
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
  std::string __str__() const {
    std::stringstream str;
    str << "X-rot: " << angleX() << " Y-rot: " << angleY()
        << " Z-rot: " << angleZ();
    return str.str();
  }
};

}  // namespace tenviz
