#include "gl_shader.hpp"

#include <fstream>
#include <iostream>
#include <regex>

#include <boost/filesystem.hpp>
//#include <filesystem>

#include "gl_error.hpp"
#include "read_file.hpp"

using namespace std;
namespace fs = boost::filesystem;
// namespace fs = std::filesystem;
namespace tenviz {

GLShader::GLShader(GLenum shader_type) {
  is_compiled_ = false;
  dirty_ = false;
  shader_id_ = glCreateShader(static_cast<GLenum>(shader_type));
  GLCheckError();
  last_read_time_ = 0;
}

GLShader::~GLShader() {
  glDeleteShader(shader_id_);
  GLCheckError();

  //  if (fs::exists(log_output_)) {
  // fs::remove(log_output_);
  //}
}

void GLShader::SetFile(const string &shader_filename) {
  shader_filename_ = shader_filename;
  log_output_ = shader_filename_ + ".out";
  dirty_ = true;
}

void GLShader::WriteLog(const std::string &log) {
  ofstream output(log_output_.c_str());
  output << log;
  output.close();
}

bool GLShader::CompileFromSource(const std::string &source_text) {
  dirty_ = true;
  log_output_ = boost::filesystem::unique_path().string();
  return CompileFromSourceImpl(source_text.c_str());
}

bool GLShader::CompileSourceFileImpl(const string &filename) {
  readfile::ScopedFileData file_data(readfile::ReadFile(filename, true));
  if (file_data.data == nullptr) {
    std::stringstream stream;
    stream << "File " << filename << " is empty";
    WriteLog(stream.str());
    return false;
  }
  const char *data = reinterpret_cast<char *>(file_data.data);

  dirty_ = false;
  return CompileFromSourceImpl(data);
}

// static std::regex _filename_regex("^\\d+");

bool GLShader::CompileFromSourceImpl(const char *source_text) {
  glShaderSource(shader_id_, 1, &source_text, NULL);
  GLCheckError();

  glCompileShader(shader_id_);
  GLCheckError();

  GLint compiled;
  glGetShaderiv(shader_id_, GL_COMPILE_STATUS, &compiled);
  GLCheckError();

  if (compiled) {
    is_compiled_ = true;
    return true;
  }

  GLint info_len;
  glGetShaderiv(shader_id_, GL_INFO_LOG_LENGTH, &info_len);
  GLCheckError();

  string raw_log;
  raw_log.resize(info_len, '\0');

  glGetShaderInfoLog(shader_id_, info_len, NULL, &raw_log[0]);
  GLCheckError();

  last_log_ = raw_log;
  // last_log_.clear();
  // regex_replace(std::back_inserter(last_log_), raw_log.begin(),
  // raw_log.end(), _filename_regex,
  // fs::path(shader_filename_).filename().string());

  return false;
}

GLShader::RecompileStatus GLShader::Recompile() {
  if (!fs::exists(shader_filename_)) {
    std::cerr << "File " << shader_filename_ << " does not exists." << endl;

    return RecompileStatus(false, false);
  }

  const time_t write_time = fs::last_write_time(shader_filename_);
  if (last_read_time_ == write_time) {
    return RecompileStatus(false, is_compiled_);
  }

  last_read_time_ = write_time;

  if (!CompileSourceFileImpl(shader_filename_)) {
    WriteLog(get_log());
    return RecompileStatus(true, false);
  } else {
    WriteLog("");
    return RecompileStatus(true, true);
  }
}
}  // namespace tenviz
