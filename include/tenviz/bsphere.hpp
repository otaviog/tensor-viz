#pragma once

#include <torch/torch.h>
#include "eigen_common.hpp"

namespace pybind11 {
class module;
}

namespace tenviz {
/**
 * Bounding sphere. It's a center 3D point and radius value.
 */
class BSphere {
 public:
  /**
   * Constructor.
   *
   * @param center The sphere's center point.
   * @param radius The sphere's radius. Pass negative values to represent an
   * empty sphere.
   */
  BSphere(const Eigen::Vector3f &center = Eigen::Vector3f(0.0f, 0.0f, 0.0f),
          float radius = -1.0)
      : center_(center), radius_(radius) {}

  /**
   * Create the sphere from an array of points.
   *
   * @param points An tensor of 3D points with shape (N x 3)
   */
  static BSphere FromPoints(const torch::Tensor &points);

  /**
   * Register the class whitin Python.
   */
  static void RegisterPybind(pybind11::module &m);

  /**
   * @return The sphere's radius.
   */
  float get_radius() const { return radius_; }

  /**
   * @return The sphere's center.
   */
  Eigen::Vector3f get_center() const { return center_; }

  /**
   * @return Whatever if the sphere is empty.
   */
  bool empty() const { return radius_ < 0.0f; }

  /**
   * Transforms the bounding sphere by an affine transformation.
   * @param transform The Affine transformation.
   * @return A new and transformed bounding sphere.
   */
  BSphere Transform(const Eigen::Affine3f &transform) const;

  /**
   * Unites with other bounding sphere, whatever if one of the two spheres are
   * totally inside, partially inside or totally separated.
   *
   * @param rhs the other bounding sphere.
   *
   * @return A new united bounding sphere.
   */
  BSphere Union(const BSphere &sphere) const;

  /**
   * Tests whatever if a point is inside it.
   * @param point test point.
   * @return True if it's inside.
   */
  bool Contains(const Eigen::Vector3f &point);

 private:
  void Add(const Eigen::Vector3f &pos);

  Eigen::Vector3f center_;
  float radius_;
};
}  // namespace tenviz
