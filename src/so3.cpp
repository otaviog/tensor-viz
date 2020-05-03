#include "so3.hpp"

#include "pose.hpp"

using namespace std;
namespace py = pybind11;

namespace tenviz {

void SO3::RegisterPybind(pybind11::module &m) {
  py::class_<SO3>(m, "SO3")
      .def(py::init<>())
      .def_static("from_matrix", &SO3::FromMatrix)
      .def_static("from_quat_pos", &SO3::FromQuaternion)
      .def("to_matrix", &SO3::ToMatrix)
      .def("get_params", &SO3::torch_params)
      .def("log", &SO3::torch_log)
      .def_static("exp", &SO3::torch_exp)
      .def("interpolate", &SO3::Interpolate)
      .def("quaternion", &SO3::torch_quaternion)
      .def("__str__", &SO3::__str__)
      .def("__repr__", &SO3::__str__);
}

}  // namespace tenviz
