#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <Eigen/Eigen>
#pragma GCC diagnostic pop

#include <torch/torch.h>

namespace tenviz {
/**
 * Construct a 3D vector from a Tensor. The template type does not
 * need to reflect the tensor's dtype.
 */
template <typename Scalar>
inline Eigen::Matrix<Scalar, 3, 1> from_tensorv3(const torch::Tensor &tensor) {
  return AT_DISPATCH_ALL_TYPES(tensor.scalar_type(), "from_tensorv3", ([&] {
                                 auto acs = tensor.accessor<scalar_t, 1>();
                                 return Eigen::Matrix<Scalar, 3, 1>(
                                     acs[0], acs[1], acs[2]);
                               }));
}

/**
 * Construct a 4x4 matrix from a Tensor. The template type does not
 * need to reflect the tensor's dtype.
 */
template <typename Scalar>
inline Eigen::Matrix<Scalar, 4, 4> from_tensorm4(const torch::Tensor &tensor) {
  Eigen::Matrix<Scalar, 4, 4> mat;
  AT_DISPATCH_ALL_TYPES(tensor.scalar_type(), "from_tensorm4", ([&] {
                          const auto acs = tensor.accessor<scalar_t, 2>();

                          mat << acs[0][0], acs[0][1], acs[0][2], acs[0][3],
                              acs[1][0], acs[1][1], acs[1][2], acs[1][3],
                              acs[2][0], acs[2][1], acs[2][2], acs[2][3],
                              acs[3][0], acs[3][1], acs[3][2], acs[3][3];
                        }));

  return mat;
}

/**
 * Construct a 3x3 matrix from a Tensor. The template type does not
 * need to reflect the tensor's dtype.
 */
template <typename Scalar>
inline Eigen::Matrix<Scalar, 3, 3> from_tensorm3(const torch::Tensor &tensor) {
  Eigen::Matrix<Scalar, 3, 3> mat;
  AT_DISPATCH_ALL_TYPES(tensor.scalar_type(), "from_tensorm3", ([&] {
                          const auto acs = tensor.accessor<scalar_t, 2>();

                          mat << acs[0][0], acs[0][1], acs[0][2], acs[1][0],
                              acs[1][1], acs[1][2], acs[2][0], acs[2][1],
                              acs[2][2];
                        }));

  return mat;
}
}  // namespace tenviz
