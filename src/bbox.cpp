#include "bbox.hpp"

#include <numeric>

#include <torch/csrc/utils/pybind.h>

namespace tenviz {
using namespace std;

BBox3D BBox3D::FromPoints(const torch::Tensor &points) {
  auto mi = std::get<0>(points.min(0));
  auto ma = std::get<0>(points.max(0));

  return BBox3D(from_tensorv3<float>(mi.cpu()), from_tensorv3<float>(ma.cpu()));
}

void BBox3D::RegisterPybind(pybind11::module &m) {
  pybind11::class_<BBox3D>(m, "BBox")
      .def_property("min", &BBox3D::get_min, nullptr)
      .def_property("max", &BBox3D::get_max, nullptr);
}

BBox3D::BBox3D()
    : min_pt_(numeric_limits<float>::infinity(),
              numeric_limits<float>::infinity(),
              numeric_limits<float>::infinity()),
      max_pt_(-numeric_limits<float>::infinity(),
              -numeric_limits<float>::infinity(),
              -numeric_limits<float>::infinity()) {}

BBox3D BBox3D::Union(const BBox3D &rhs) const {
  return BBox3D(Eigen::Vector3f(min(min_pt_[0], rhs.min_pt_[0]),
                                min(min_pt_[1], rhs.min_pt_[1]),
                                min(min_pt_[2], rhs.min_pt_[2])),
                Eigen::Vector3f(max(max_pt_[0], rhs.max_pt_[0]),
                                max(max_pt_[1], rhs.max_pt_[1]),
                                max(max_pt_[2], rhs.max_pt_[2])));
}

BBox3D BBox3D::Transform(const Eigen::Affine3f &transform) const {
  return BBox3D(transform * min_pt_, transform * max_pt_);
}
}  // namespace tenviz
