#pragma once

#include "eigen_common.hpp"

namespace tenviz {
namespace math {

const float kFloatPI = 3.14159265358979323846f;
const double kPI = 3.14159265358979323846;

/* Converts degrees to radians.
 * @deg the angle in degrees
 * @return the angle in radians
 */
inline float DegToRad(float deg) {
  static const float PI_D_180 = kFloatPI / 180.0f;
  return deg * PI_D_180;
}

/* Converts radians to degrees.
 * @rad the angle in radians
 * @return the angle in degrees
 */
inline float RadToDeg(float rad) {
  static const float _180_D_PI = 180.0f / kFloatPI;
  return rad * _180_D_PI;
}

/* Converts degrees to radians.
 * @deg the angle in degrees
 * @return the angle in radians
 */
inline double DegToRad(double deg) {
  static const double PI_D_180 = kFloatPI / 180.0f;
  return deg * PI_D_180;
}

/* Converts radians to degrees.
 * @rad the angle in radians
 * @return the angle in degrees
 */
inline double RadToDeg(double rad) {
  static const double _180_D_PI = 180.0f / kFloatPI;
  return rad * _180_D_PI;
}

/**
 * Compute the inverse of the transpose of an affine transformation
 * matrix for correct normal vector transformations.
 *
 * @param transformation 4x4 affine transformation matrix.
 *
 * @return A 3x3 matrix for transforming normals.
 */
Eigen::Matrix3f ComputeNormalModelview(const Eigen::Matrix4f &transformation);

}
}
