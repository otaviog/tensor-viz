#include "bsphere.hpp"

#include <torch/csrc/utils/pybind.h>

namespace tenviz {

BSphere BSphere::FromPoints(const torch::Tensor &points) {
  auto center = points.mean(0);

  float max_radius = torch::norm(points - center, 2, 1).max().item().toFloat();

  return BSphere(from_tensorv3<float>(center.cpu()), max_radius);
}

void BSphere::RegisterPybind(pybind11::module &m) {
  pybind11::class_<BSphere>(m, "BSphere")
      .def_property("center", &BSphere::get_center, nullptr)
      .def_property("radius", &BSphere::get_radius, nullptr);
}

BSphere BSphere::Transform(const Eigen::Affine3f &transform) const {
  const auto matrix = transform.matrix();
  const Eigen::Vector3f col0(matrix(0, 0), matrix(1, 0), matrix(2, 0));
  const Eigen::Vector3f col1(matrix(0, 1), matrix(1, 1), matrix(2, 1));
  const Eigen::Vector3f col2(matrix(0, 2), matrix(1, 2), matrix(2, 2));

  return BSphere(transform * center_,
                 radius_ * std::sqrt(std::max(col0.squaredNorm(),
                                              std::max(col1.squaredNorm(),
                                                       col2.squaredNorm()))));
}

void BSphere::Add(const Eigen::Vector3f &point) {
  Eigen::Vector3f vec = point - center_;
  const float dist = vec.norm();
  if (dist > radius_) {
    const float half_dist = (dist - radius_) * 0.5f;
    center_ += vec * half_dist / dist;
    radius_ += half_dist;
  }
}

BSphere BSphere::Union(const BSphere &sphere) const {
  if (empty()) {
    return sphere;
  }
  if (sphere.empty()) {
    return *this;
  }

  Eigen::Vector3f dist_point =
      (sphere.get_center() - center_).normalized() * sphere.get_radius();

  BSphere new_sphere(*this);
  new_sphere.Add(sphere.get_center() + dist_point);
  new_sphere.Add(sphere.get_center() - dist_point);

  return new_sphere;
}

bool BSphere::Contains(const Eigen::Vector3f &point) {
  const float norm = (point - center_).norm();
  return norm < radius_;
}

}  // namespace tenviz
