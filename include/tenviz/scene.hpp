#pragma once

#include <memory>
#include <set>

#include "eigen_common.hpp"

#include "anode.hpp"

namespace tenviz {

/**
 * Composite of nodes.
 */
class Scene : public ANode {
 public:
  static void RegisterPybind(
      pybind11::module &m,
      pybind11::class_<ANode, std::shared_ptr<ANode>> &anode);

  /**
   * Add a node into the scene.
   */
  void Add(std::shared_ptr<ANode> node);

  /**
   * Remove a node from the scene.
   */
  void Erase(std::shared_ptr<ANode> node);

  /**
   * Remove all nodes from the scene.
   */
  void Clear();

  /**
   * Send the rendering commands.
   *
   * @param projection Projection matrix.
   * @param camera Camera view matrix.
   */
  void Draw(const Eigen::Matrix4f &projection,
            const Eigen::Matrix4f &camera) override;

  /**
   * Computes the scene bounding by uniting all subnodes bounds.
   *
   * @return The scene bounds.
   */
  Bounds GetBounds() const override;

 private:
  std::set<std::shared_ptr<ANode>> nodes_;
};
}  // namespace tenviz
