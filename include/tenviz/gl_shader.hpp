#pragma once

#include <ctime>
#include <string>

#include "gl_common.hpp"

namespace tenviz {
class GLShader {
 public:
  struct RecompileStatus {
    RecompileStatus(bool recompiled = false, bool success = false)
        : recompiled(recompiled), success(success) {}
    bool recompiled;
    bool success;
  };

  GLShader(GLenum shader_type);

  ~GLShader();

  GLShader(const GLShader &copy) = delete;

  GLShader &operator=(const GLShader &copy) = delete;

  GLuint get_shader_id() { return shader_id_; }

  const std::string &get_shader_filename() const { return shader_filename_; }

  void SetFile(const std::string &shader_filename);

  bool CompileFromSource(const std::string &source_text);

  /**
   * Checks if the shader file was changed, and if is the case, then
   * recompile it.
   *
   * @return Recompilation status.
   */
  RecompileStatus Recompile();

  bool is_compiled() const { return is_compiled_; }

  const std::string &get_log() const { return last_log_; }

 private:
  void WriteLog(const std::string &log);

  bool CompileSourceFileImpl(const std::string &filename);

  bool CompileFromSourceImpl(const char *source_text);

  GLuint shader_id_;
  std::string shader_filename_;
  std::time_t last_read_time_;
  bool dirty_, is_compiled_;
  std::string last_log_, log_output_;
};
}  // namespace tenviz
