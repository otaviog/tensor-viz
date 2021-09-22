#include "style.hpp"

#include <torch/csrc/utils/pybind.h>

namespace tenviz {
Style::Style() {
  line_width = 1.0f;
  point_size = 1.0f;
  polygon_mode = PolygonMode::kFill;
  alpha_blending = false;

  polygon_offset_mode = PolygonOffsetMode::kNone;
  polygon_offset_factor = 0.0f;
  polygon_offset_units = 0.0f;
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

  if (polygon_offset_mode == PolygonOffsetMode::kNone) {
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glDisable(GL_POLYGON_OFFSET_POINT);
  } else {
    glEnable(static_cast<GLenum>(polygon_offset_mode));
    glPolygonOffset(polygon_offset_factor, polygon_offset_units);
  }
}

void Style::RegisterPybind(pybind11::module &m) {
  pybind11::enum_<PolygonMode>(m, "PolygonMode")
      .value("Fill", PolygonMode::kFill)
      .value("Wireframe", PolygonMode::kWireframe)
      .value("Point", PolygonMode::kPoint)
      .export_values();

  pybind11::enum_<PolygonOffsetMode>(m, "PolygonOffsetMode")
      .value("None", PolygonOffsetMode::kNone)
      .value("Fill", PolygonOffsetMode::kFill)
      .value("Line", PolygonOffsetMode::kLine)
      .value("Point", PolygonOffsetMode::kPoint)
      .export_values();

  pybind11::class_<Style>(m, "Style")
      .def_readwrite("polygon_mode", &Style::polygon_mode)
      .def_readwrite("line_width", &Style::line_width)
      .def_readwrite("point_size", &Style::point_size)
      .def_readwrite("alpha_blending", &Style::alpha_blending)
      .def_readwrite("polygon_offset_mode", &Style::polygon_offset_mode)
      .def_readwrite("polygon_offset_factor", &Style::polygon_offset_factor)
      .def_readwrite("polygon_offset_units", &Style::polygon_offset_units);
}
}  // namespace tenviz
