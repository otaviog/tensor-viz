#include "pose.hpp"

namespace tenviz {
void Pose::RegisterPybind(pybind11::module &m) {
  py::class_<Pose>(m, "Pose")
      .def(py::init<>())
      .def(py::init<float, float, float, float, float, float, float>())
      .def_static("from_matrix", &Pose::FromMatrix)
      .def_static("from_rotation_matrix_translation",
                  &Pose::FromRotationMatrixTranslation)
      .def_static("from_se3", &Pose::FromSE3)
      .def("to_matrix", &Pose::ToMatrix)
      .def("inverse", &Pose::Inverse)
      .def("interpolate", &Pose::Interpolate)
      .def("get_euler_angles", &Pose::get_euler_angles)
      .def("get_quaternion", &Pose::get_quaternion)
      .def("get_translation", &Pose::get_translation)
      .def("__matmul__", &Pose::operator*)
      .def("__str__", &Pose::__str__)
      .def("__repr__", &Pose::__str__);
}
}  // namespace tenviz
