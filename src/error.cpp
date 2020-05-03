#include "error.hpp"

#include <torch/csrc/utils/pybind.h>

using namespace std;

namespace tenviz {
void Error::RegisterPybind(pybind11::module &m) {
  py::register_exception<Error>(m, "Error");
}
}  // namespace tenviz
