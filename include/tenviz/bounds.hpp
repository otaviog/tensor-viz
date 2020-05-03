#pragma once

#include <torch/torch.h>

#include "bbox.hpp"
#include "bsphere.hpp"

namespace pybind11 {
class module;
}

namespace tenviz {

/**
 * Requested 3D boundaries for scene nodes, which is an axis aligned
 * bounding box (BBox3D) and a bounding sphere (BSphere).
 */
class Bounds {
 public:

  /**
   * Create the boundaries from an array of 3D points.
   *
   * @param points An tensor of 3D points with shape (N x 3)
   */
  static Bounds FromPoints(const torch::Tensor &points) {
    return Bounds(BBox3D::FromPoints(points), BSphere::FromPoints(points));
  }

  /**
   * Creates an empty boundary.
   */
  static Bounds UnitBounds() {
    return Bounds(BBox3D(Eigen::Vector3f(-1.0f, -1.0f, -1.0f),
                         Eigen::Vector3f(1.0f, 1.0f, 1.0f)),
                  BSphere(Eigen::Vector3f(0.0f, 0.0f, 0.0f), 1.0f));
  }

  /**
   * Register the class whitin Python.
   */
  static void RegisterPybind(pybind11::module &m);

  /**
   * Constructs an empty boundary.
   */
  Bounds(const BBox3D &box = BBox3D(), const BSphere &sphere = BSphere())
      : box_(box), sphere_(sphere) {}

  /**
   * Transforms the boundary an affine transformation.  @param
   * transform the Affine transformation.  @return a new and
   * transformed bounding.
   */   
  Bounds Transform(const Eigen::Affine3f &transform) const {
    return Bounds(box_.Transform(transform), sphere_.Transform(transform));
  }

  /**
   * @return The boundary's axis aligned bounding box
   */
  BBox3D get_box() const { return box_; }

  /**
   * @return The bondary's bounding sphere.
   */
  BSphere get_sphere() const { return sphere_; }

  /**
   * Unites with other boundary, whatever if one of the two boxes are
   * totally inside, partially inside or totally separated.
   *
   * @param rhs The other bounds
   *
   * @return The united bounds.
   */
  Bounds Union(const Bounds &bounds) const {
    return Bounds(box_.Union(bounds.get_box()),
                  sphere_.Union(bounds.get_sphere()));
  }

 private:
  BBox3D box_;
  BSphere sphere_;
};
}  // namespace tenviz
