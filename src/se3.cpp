#include "se3.hpp"

#include "pose.hpp"

using namespace std;
namespace py = pybind11;

namespace tenviz {

void SE3::RegisterPybind(pybind11::module &m) {
  py::class_<SE3>(m, "SE3")
      .def(py::init<>())
      .def_static("from_matrix", &SE3::FromMatrix)
      .def_static("from_rot_pos", &SE3::FromRotationMatrixTranslation)
      .def_static("from_quat_pos", &SE3::FromQuaternionTranslation)
      .def("to_matrix", &SE3::ToMatrix)
      .def("get_params", &SE3::torch_params)
      .def("log", &SE3::torch_log)
      .def_static("exp", &SE3::torch_exp)
      .def("interpolate", &SE3::Interpolate)
      .def("angle_x", &SE3::angleX)
      .def("angle_y", &SE3::angleY)
      .def("angle_z", &SE3::angleZ)
      .def("translation", &SE3::torch_translation)
      .def("quaternion", &SE3::torch_quaternion)
      .def("__str__", &SE3::__str__)
      .def("__repr__", &SE3::__str__);
}

SE3 SE3::FromPose(const Pose &pose) {
  return SE3(pose.quaternion(), pose.translation());
}

}  // namespace tenviz
