#pragma once

#include "bbox.hpp"
#include "bsphere.hpp"
#include "projection.hpp"

namespace tenviz {

/* This class contains the standard camera format, like in the
 * gluLookAt format, with eye position, view and up vectors.
 */
class Camera {
 public:
  /* Constructor.
   *
   * @pos eye position (default is zero vector).
   * @view view vector (default is [0.0f, 0.0f, -1.0f]).
   * @up up vector (default is [0.0f, 1.0f, 0.0f]).
   */
  Camera(const Eigen::Vector3f &eye = Eigen::Vector3f(0.0f, 0.0f, 1.0f),
         const Eigen::Vector3f &view = Eigen::Vector3f(0.0f, 0.0f, -1.0f),
         const Eigen::Vector3f &up = Eigen::Vector3f(0.0f, 1.0f, 0.0f));
  /**
   * Creates a camera using sphere coordinates around a center
   * at a given distance.

   * @param elevation elevation coordinate in degree [0, 180].
   * @param azimuth azimuth coordinate in degree [0, 360].
   * @param center center of rotation.
   * @param distance distance from the center.
   * @return a camera with the above position and view.
   */
  static Camera FromSphereCoordinates(float elevation, float azimuth,
                                      const Eigen::Vector3f &center,
                                      float distance);

  static Camera FromViewMatrix(const Eigen::Matrix4f &matrix);

  /* Rotates the view point around the X axis.
   * @angle angle in radians
   */
  void RotateRight(float angle);

  /* Rotates the view point around the Y axis.
   * @angle angle in radians
   */
  void RotateUp(float angle);

  /* Rotates the view point around the Z axis.
   * @angle angle in radians
   */
  void RotateView(float angle);

  void RotateAxis(const Eigen::Vector3f &axis, float angle);

  void RotateQuat(const Eigen::Quaternionf &quat);

  /* Moves the eye and view. The direction is defined by view - eye.
   * @ac the amount of displacement.
   */
  void Forward(float ac);

  /* Strife to relative left (-) or right (+).
   * @param ac amount of displacement, negative values means to move to the
   * left.
   */
  void Strife(float ac);

  /* Generates a matrix with camera transformations.
   * @return the matrix
   */
  Eigen::Matrix4f GetMatrix() const;

  void Move(const Eigen::Vector3f &move);

  /* Sets camera values.
   * @pos eye position (default is zero vector).
   * @view view vector (default is [0.0f, 0.0f, -1.0f]).
   * @up up vector (default is [0.0f, 1.0f, 0.0f]).
   */
  void set(Eigen::Vector3f eye, Eigen::Vector3f view, Eigen::Vector3f up) {
    set_eye(eye);
    set_view(view);
    set_up(up);
  }

  /* Sets eye position
   */
  void set_eye(const Eigen::Vector3f &eye) { eye_ = eye; }

  /* Sets view position.
   */
  void set_view(const Eigen::Vector3f &view) { view_ = view; }

  /* Sets up vector.
   */
  void set_up(const Eigen::Vector3f &up) { up_ = up; }

  /* Returns eye position.
   */
  const Eigen::Vector3f &get_eye() const { return eye_; }

  /* Returns view position.
   */
  const Eigen::Vector3f &get_view() const { return view_; }

  /* Returns up vector.
   */
  const Eigen::Vector3f &get_up() const { return up_; }

  const Eigen::Vector3f GetRight() const {
    return view_.cross(up_).normalized();
  }

  /* Returns normalied camera direction vector.
   */
  Eigen::Vector3f get_direction() const { return (view_ - eye_).normalized(); }

 private:
  Eigen::Vector3f up_, view_, eye_;
  Eigen::Vector3f world_up_;
};

namespace camera {

float GetMinBoundingDistance(float radius, float fov_y);

void GetSceneFrustumFitTransform(const BSphere &scene_sphere, float fov_y,
                                 float &near, float &distance);

inline Eigen::Matrix4f GetSphereView(float elevation, float azimuth,
                                       const Eigen::Vector3f &center,
                                       float distance) {
  return Camera::FromSphereCoordinates(elevation, azimuth, center, distance)
      .GetMatrix();
}
}  // namespace camera

}  // namespace tenviz
