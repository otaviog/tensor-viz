#pragma once

#include <map>
#include <memory>
#include <string>

#include <torch/csrc/utils/pybind.h>
#include <torch/torch.h>

#include "anode.hpp"
#include "gl_common.hpp"
#include "style.hpp"

namespace tenviz {
enum MatPlaceholder {
  kModelview,
  kProjection,
  kProjectionModelview,
  kNormalModelview,
  kObject
};

enum DrawMode {
  kQuads = GL_QUADS,
  kTriangles = GL_TRIANGLES,
  kLines = GL_LINES,
  kPoints = GL_POINTS
};

class GLShaderProgram;
class GLBuffer;
class GLTexture;

class DrawProgram : public ANode {
 public:
    static void RegisterPybind(
      pybind11::module &m,
      pybind11::class_<ANode, std::shared_ptr<ANode>> &anode);
  
  DrawProgram(DrawMode mode, std::shared_ptr<GLShaderProgram> program,
              bool ignore_missing = false);

  void Draw(const Eigen::Matrix4f &projection,
            const Eigen::Matrix4f &view) override;

  void SetBounds(const torch::Tensor &points);

  Bounds GetBounds() const override {
    return bounds_.Transform(Eigen::Affine3f(transform));
  }

  void SetItem(const std::string &name, std::shared_ptr<GLBuffer> buffer);

  void SetItem(const std::string &name, const torch::Tensor &tensor);

  void SetItem(const std::string &name, MatPlaceholder placeholder);

  void SetItem(const std::string &name, std::shared_ptr<GLTexture> texture);

  void SetItem(const std::string &name, float value);

  void SetItem(const std::string &name, int value);

  pybind11::object GetItem(const std::string &name);

  std::shared_ptr<GLBuffer> indices;

  std::shared_ptr<DrawProgram> Clone();
  
  Style style;

  void SetMaxDrawElems(int max_draw_elems) {
    max_draw_elems_ = max_draw_elems;
  }
  
 private:
  std::shared_ptr<GLShaderProgram> program_;
  std::map<std::string, std::shared_ptr<GLBuffer>> buffers_;
  std::map<std::string, MatPlaceholder> matrix_placeholders_;
  std::map<std::string, torch::Tensor> uniforms_;
  std::map<std::string, std::shared_ptr<GLTexture>> textures_;
  Bounds bounds_;
  DrawMode draw_mode_;

  GLuint vao_;
  bool ignore_missing_;
  int max_draw_elems_;
};
}  // namespace tenviz
