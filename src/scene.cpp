#include "scene.hpp"

#include "bounds_glrender.hpp"

using namespace std;

namespace tenviz {

void Scene::RegisterPybind(
    pybind11::module &m,
    pybind11::class_<ANode, std::shared_ptr<ANode>> &anode) {
  py::class_<Scene, shared_ptr<Scene>>(m, "Scene", anode)
      .def(py::init())
      .def("add", &Scene::Add)
      .def("erase", &Scene::Erase)
      .def("clear", &Scene::Clear)
      .def("get_bounds", &Scene::GetBounds);
}

void Scene::Add(shared_ptr<ANode> node) { nodes_.insert(node); }

void Scene::Erase(shared_ptr<ANode> node) { nodes_.erase(node); }

void Scene::Clear() { nodes_.clear(); }

void Scene::Draw(const Eigen::Matrix4f &projection,
                 const Eigen::Matrix4f &_view) {
  const Eigen::Matrix4f view = _view * transform;
  for (shared_ptr<ANode> node : nodes_) {
    if (node->visible) {
      node->Draw(projection, view);
    }

    const Bounds &bounds = node->GetBounds();
    if (node->draw_bsphere) {
      DrawSphere(bounds.get_sphere(), projection, view);
    }

    if (node->draw_bbox) {
      // DrawBox // TODO
    }
  }
}

Bounds Scene::GetBounds() const {
  Bounds bounds;

  for (shared_ptr<ANode> node : nodes_) {
    bounds = bounds.Union(node->GetBounds());
  }

  return bounds.Transform(Eigen::Affine3f(transform));
}

}  // namespace tenviz
