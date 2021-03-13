#include "style.hpp"

#include <torch/csrc/utils/pybind.h>

namespace tenviz {
Style::Style() {
  line_width = 1.0f;
  point_size = 1.0f;
  polygon_mode = kFill;
  alpha_blending = false;
}

void Style::Activate() const {
  glPolygonMode(GL_FRONT_AND_BACK, static_cast<GLenum>(polygon_mode));
  glLineWidth(line_width);
  glPointSize(point_size);

  if (alpha_blending) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
  } else {
    glDisable(GL_BLEND);
  }
  
}

void Style::RegisterPybind(pybind11::module &m) {
  pybind11::enum_<PolygonMode>(m, "PolygonMode")
      .value("Fill", PolygonMode::kFill)
      .value("Wireframe", PolygonMode::kWireframe)
      .value("Point", PolygonMode::kPoint)
      .export_values();

  pybind11::class_<Style>(m, "Style")
      .def_readwrite("polygon_mode", &Style::polygon_mode)
      .def_readwrite("line_width", &Style::line_width)
      .def_readwrite("point_size", &Style::point_size)
      .def_readwrite("alpha_blending", &Style::alpha_blending);
}
}  // namespace tenviz
