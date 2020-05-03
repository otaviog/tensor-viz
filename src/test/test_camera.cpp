#include "catch.hpp"

#include <tenviz/camera.hpp>

TEST_CASE("Camera", "[ToMatrix/FromMatrix]") {
  Eigen::Vector3f eye(0.960158, 0.911863, 0.29514);
  Eigen::Vector3f view(-0.178538, -0.245247, 0.952879);
  Eigen::Vector3f up(-0.0451653, 0.969461, 0.241052);
  tenviz::Camera camera(eye, view, up);

  const Eigen::Matrix4f view_matrix = camera.GetMatrix();

  tenviz::Camera other = tenviz::Camera::FromViewMatrix(view_matrix);

  const Eigen::Vector3f other_eye(other.get_eye());
  CHECK(eye[0] == Approx(other_eye[0]));
  CHECK(eye[1] == Approx(other_eye[1]));
  CHECK(eye[2] == Approx(other_eye[2]));

  CHECK(view[0] == Approx(other.get_view()[0]));
  CHECK(view[1] == Approx(other.get_view()[1]));
  CHECK(view[2] == Approx(other.get_view()[2]));

  CHECK(up[0] == Approx(other.get_up()[0]));
  CHECK(up[1] == Approx(other.get_up()[1]));
  CHECK(up[2] == Approx(other.get_up()[2]));
}

#if 0
TEST_CASE("Camera", "[ToMatrix/FromMatrix]") {
  Eigen::Matrix4f mtx;
  mtx << -0.988051, 5.58794e-09, -0.154127, 0.465902, -0.0235515, 0.988256,
      0.15098, -0.989622, 0.152317, 0.152806, -0.976448, 0.294805, 0, 0, 0, 1;

  tenviz::Camera cam(tenviz::Camera::FromViewMatrix(vi
}
#endif
