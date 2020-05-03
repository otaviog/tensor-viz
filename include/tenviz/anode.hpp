#pragma once

#include <torch/csrc/utils/pybind.h>
#include <torch/torch.h>

#include "bounds.hpp"
#include "eigen_common.hpp"

namespace tenviz {

/**
 * Base class for all TensorViz's geometric entities.
 */
class ANode {
 public:
  static pybind11::class_<ANode, std::shared_ptr<ANode>> RegisterPybind(
      pybind11::module &m);

  ANode() {
    visible = true;
    transform = Eigen::Matrix4f::Identity();
    draw_bsphere = false;
    draw_bbox = false;
  }

  virtual ~ANode() {}

  /**
   * Derived class should implement its OpenGL calls for rendering,
   * using the given projection and view matrices.
   *
   * @param projection A (4x4) float matrix corresponding to an
   * OpenGL's projection matrix.  @param view A (4x4) float matrix
   * corresponding to an OpenGL's view matrix. This matrix should be
   * multiplied by ANode::transform, to obtain the Modelview matrix.
   */
  virtual void Draw(const Eigen::Matrix4f &projection,
                    const Eigen::Matrix4f &view) = 0;

  /**
   * Derived class should handle computing its boundaries. This is
   * used for culling geometry out of rendering if not viewed by the
   * camera.
   *
   * @return The geometry boundary.
   */
  virtual Bounds GetBounds() const = 0;

  Eigen::Matrix4f transform; /**< Node's transformation
                              * matrix. Default is Identity.*/
  bool visible;      /**< Whatever if the node should be rendered. Default
                      * is true.*/
  bool draw_bsphere; /**< Whatever if its bounding sphere should be
                        draw for debugging. Default is false.*/
  bool draw_bbox;    /**<Whatever if its bounding box should be draw for
                      * debugging. Default is false.*/
};
}  // namespace tenviz
