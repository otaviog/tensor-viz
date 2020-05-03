#include "anode.hpp"

#include <pybind11/eigen.h>

namespace tenviz {

pybind11::class_<ANode, std::shared_ptr<ANode>> ANode::RegisterPybind(
    pybind11::module &m) {
  pybind11::class_<ANode, std::shared_ptr<ANode>> node(m, "ANode");
  node.def_readwrite("transform", &ANode::transform)
      .def_readwrite("draw_sphere", &ANode::draw_bsphere)
      .def_readwrite("draw_box", &ANode::draw_bbox)
      .def_readwrite("visible", &ANode::visible);

  return node;
}

}  // namespace tenviz
