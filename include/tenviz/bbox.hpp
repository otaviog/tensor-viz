#pragma once

#include <torch/torch.h>
#include <torch/csrc/utils/pybind.h>
#include "eigen_common.hpp"


namespace tenviz {
/**
 * Axis Aligned Bounding Box for 3D spaces.
 */
class BBox3D {
 public:
  /**
   * Create the box from the bounding minimum and maximum of an array
   * of points.
   *
   * @param points An tensor of 3D points with shape (N x 3)
   */
  static BBox3D FromPoints(const torch::Tensor &points);

  /**
   * Register the class whitin Python.
   */
  static void RegisterPybind(pybind11::module &m);

  /**
   * Constructs an empty box. Minimum and maximum are set to invalid
   * values. Operation like BBox3D::Union are going to detect this and
   * return the appropriate results.
   */
  BBox3D();

  /**
   * Constructs from given minimum and maximum.
   *
   * @param min_pt The minimum point.
   * @param max_pt The maximum point.
   */
  BBox3D(const Eigen::Vector3f &min_pt, const Eigen::Vector3f &max_pt)
      : min_pt_(min_pt), max_pt_(max_pt) {}

  /**
   *
   * @return The minimum point, i.e., the left, bottom and near point.
   */
  Eigen::Vector3f get_min() const { return min_pt_; }

  /**
   *
   * @return The minimum point, i.e., the left, bottom and near point.
   */
  Eigen::Vector3f get_max() const { return max_pt_; }

  /**
   *@return The box's center point.
   */
  Eigen::Vector3f get_center() const { return (min_pt_ + max_pt_) * 0.5f; }

  /**
   * Unites with other bounding box, whatever if one of the two boxes are
   * totally inside, partially inside or totally separated.
   *
   * @param rhs The other bounding box.
   *
   * @return A new united bounding box.
   */
  BBox3D Union(const BBox3D &rhs) const;

  /**
   * Transforms the bounding box by an affine transformation.
   * @param transform The Affine transformation.
   * @return A new and transformed bounding box.
   */
  BBox3D Transform(const Eigen::Affine3f &transform) const;

 private:
  Eigen::Vector3f min_pt_, max_pt_;
};
}  // namespace tenviz
