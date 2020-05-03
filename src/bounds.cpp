#include "bounds.hpp"

#include <torch/csrc/utils/pybind.h>

namespace tenviz {

void Bounds::RegisterPybind(pybind11::module &m) {
  pybind11::class_<Bounds>(m, "Bounds")
      .def_property("box", &Bounds::get_box, nullptr)
      .def_property("sphere", &Bounds::get_sphere, nullptr);
}

}  // namespace tenviz
