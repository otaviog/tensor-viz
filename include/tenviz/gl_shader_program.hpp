#pragma once

#include <memory>
#include <set>
#include <vector>

#include <torch/torch.h>
#include <boost/format.hpp>

#include "context_resource.hpp"
#include "eigen_common.hpp"
#include "gl_common.hpp"
#include "gl_shader.hpp"

namespace tenviz {

class GLShaderProgram : public IContextResource {
 public:
  enum ShaderType {
    kVertex = GL_VERTEX_SHADER,
    kFragment = GL_FRAGMENT_SHADER,
    kGeometry = GL_GEOMETRY_SHADER
  };

  static std::shared_ptr<GLShaderProgram> LoadFS(
      const std::string &vertex_filepath, const std::string &frag_filepath = "",
      const std::string &geo_filepath = "");

  static void RegisterPybind(pybind11::module &m,
                             IContextResource::PythonClassDef &base_class);

  GLShaderProgram();

  ~GLShaderProgram();

  GLShaderProgram(const GLShaderProgram &copy) = delete;

  GLShaderProgram &operator=(const GLShaderProgram &copy) = delete;

  void Bind(bool bind_it);

  bool Link();

  void AddShader(ShaderType shader_type, const std::string &filename);

  void AddShaderFromSource(ShaderType shader_type,
                           const std::string &shader_source);

  void Release() override;

  int get_num_shaders() const { return static_cast<int>(shaders_.size()); }

  const std::string &get_link_log() const { return last_link_log_; }

  bool HasUniform(const std::string &name) const;

  bool HasAttrib(const std::string &name) const;

  GLint GetUniformLocation(const std::string &name);

  GLint GetAttribLocation(const std::string &name);

  void SetUniform(const std::string &name, const torch::Tensor &tensor);

  void SetUniformValue(const std::string &name, float v) {
    if (is_binded_) glUniform1f(GetUniformLocation(name), v);
  }
  void SetUniformValue(const std::string &name, int v) {
    if (is_binded_) glUniform1i(GetUniformLocation(name), v);
  }
  void SetUniformValue(const std::string &name, short v) {
    if (is_binded_) glUniform1i(GetUniformLocation(name), v);
  }
  void SetUniformValue(const std::string &name, char v) {
    if (is_binded_) glUniform1i(GetUniformLocation(name), v);
  }

  void SetUniformValue(const std::string &name, const Eigen::Vector2f &v) {
    if (is_binded_) glUniform2f(GetUniformLocation(name), v[0], v[1]);
  }
  void SetUniformValue(const std::string &name, const Eigen::Vector3f &v) {
    if (is_binded_) glUniform3f(GetUniformLocation(name), v[0], v[1], v[2]);
  }
  void SetUniformValue(const std::string &name, const Eigen::Vector4f &v) {
    if (is_binded_)
      glUniform4f(GetUniformLocation(name), v[0], v[1], v[2], v[3]);
  }
  void SetUniformValue(const std::string &name, const Eigen::Matrix3f &mat) {
    if (is_binded_)
      glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, mat.data());
  }
  void SetUniformValue(const std::string &name, const Eigen::Matrix4f &mat) {
    if (is_binded_)
      glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, mat.data());
  }

 private:
  void AddShader(std::shared_ptr<GLShader> shader);

  void WriteLog(const std::string &logr);

  GLuint program_id_;
  std::vector<std::shared_ptr<GLShader>> shaders_;
  std::string last_link_log_;
  bool is_linked_, is_binded_;

  std::set<std::string> not_found_variables_;
};
}  // namespace tenviz
