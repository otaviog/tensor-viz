#pragma once

#include "eigen_common.hpp"

namespace tenviz {
/**
 * Perspective projection frustum parameters.
 */
class Projection {
 public:
  /**
   * @param left Left position.
   * @param right Right position.
   * @param bot Bottom position.
   * @param top Top position.
   * @param near Near distance.
   * @param far Far distance.
   */
  Projection(float left, float right, float bot, float top, float near,
             float far);

  /**
   * Perspective frustum from fov and aspect.
   *
   * @param fov_y Field of view in the Y-axis.
   * @param aspect Aspect ratio
   * @param near Near distance.
   * @param far Far distance.
   */
  static Projection Perspective(float fov_y, float aspect, float near,
                                float far);

  /**
   * Construct a OpenGL's projection matrix.
   */
  Eigen::Matrix4f GetMatrix() const;

  /**
   * @return The frustum field of view.
   */
  float get_fov() const { return fov_; }

  /**
   * @return The frustum aspect ratio.
   */
  float get_aspect() const { return aspect_; }

  /**
   * @return The far plane distance.
   */
  float get_far() const { return far_; }

  /**
   * @return The near plane distance.
   */
  float get_near() const { return near_; }

  /**
   * @return The left boundary on the near plane.
   */
  float get_left() const { return left_; }

  /**
   * @return The right boundary on the near plane.
   */
  float get_right() const { return right_; }

  /**
   * @return The bottom boundary on the near plane.
   */
  float get_bottom() const { return bottom_; }

  /**
   * @return The top boundary on the near plane.
   */
  float get_top() const { return top_; }

  /**
   * @return The left boundary on the far plane.
   */
  float get_far_left() const { return far_left_; }

  /**
   * @return The right boundary on the far plane.
   */
  float get_far_right() const { return far_right_; }

  /**
   * @return The bottom boundary on the far plane.
   */
  float get_far_bottom() const { return far_bottom_; }

  /**
   * @return The top boundary on the far plane.
   */
  float get_far_top() const { return far_top_; }

 private:
  float far_, near_, left_, right_, bottom_, top_;
  float far_left_, far_right_, far_top_, far_bottom_;
  float fov_, aspect_;
};
}  // namespace tenviz
