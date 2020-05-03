#include "math.hpp"

using namespace Eigen;

namespace tenviz {
namespace math {
Matrix3f ComputeNormalModelview(const Matrix4f &modelview) {
  Matrix4f nm4 = modelview.inverse().transpose();
  Matrix3f normal_modelview;
  normal_modelview << nm4(0, 0), nm4(0, 1), nm4(0, 2), nm4(1, 0), nm4(1, 1),
      nm4(1, 2), nm4(2, 0), nm4(2, 1), nm4(2, 2);
  return normal_modelview;
}

}  // namespace math
}  // namespace tenviz
