#pragma once

#include "gl_common.hpp"

namespace pybind11 {
class module;
}

namespace tenviz {

/**
 * Mode for rendering polygon geometry.
 */
enum PolygonMode {
  kFill = GL_FILL,      /**< Filled polygons. */
  kWireframe = GL_LINE, /**< Draw edges only. */
  kPoint = GL_POINT     /**<Draw vertices only*/
};

/**
 * Drawing style properties.
 */
class Style {
 public:
  /**
   * Scoped variable for activating style and reseting to the defaults
   * after scope end.
   */
  class Scoped {
   public:
    Scoped(const Style &style) { style.Activate(); }

    ~Scoped() {
      Style default_style;
      default_style.Activate();
    }
  };

  static void RegisterPybind(pybind11::module &m);

  /**
   * Construct a style with the default options. 
   */
  Style();

  /**
   * Activate the style within the Graphics API.
   */
  void Activate() const;

  float line_width;         /**Line width for wireframe rendering.*/
  float point_size;         /**Point size for point rendering.*/
  PolygonMode polygon_mode; /**Polygon rendering mode.*/
  bool alpha_blending;
};
};  // namespace tenviz
