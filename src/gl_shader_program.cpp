#include "gl_shader_program.hpp"

#include <fstream>
#include <iostream>

#include <boost/format.hpp>

#include "gl_error.hpp"

using namespace std;

namespace tenviz {

shared_ptr<GLShaderProgram> GLShaderProgram::LoadFS(
    const string &vertex_filepath, const string &frag_filepath,
    const string &geo_filepath) {
  auto program = make_shared<GLShaderProgram>();
  if (!vertex_filepath.empty()) program->AddShader(kVertex, vertex_filepath);
  if (!frag_filepath.empty()) program->AddShader(kFragment, frag_filepath);
  if (!geo_filepath.empty()) program->AddShader(kGeometry, geo_filepath);
  IContextResource::RegisterResourceOnCurrent(program);

  // Compile the first time
  program->Bind(true);
  program->Bind(false);

  return program;
}

void GLShaderProgram::RegisterPybind(
    pybind11::module &m, IContextResource::PythonClassDef &base_class) {
  pybind11::class_<GLShaderProgram, shared_ptr<GLShaderProgram>>(
      m, "GLShaderProgram", base_class);
  m.def("load_program_fs", &GLShaderProgram::LoadFS);
}

GLShaderProgram::GLShaderProgram() {
  program_id_ = glCreateProgram();
  GLCheckError();

  is_linked_ = false;
  is_binded_ = false;
}

GLShaderProgram::~GLShaderProgram() { Release(); }

void GLShaderProgram::AddShader(ShaderType shader_type,
                                const string &filename) {
  shared_ptr<GLShader> shader(
      make_shared<GLShader>(static_cast<GLenum>(shader_type)));
  shader->SetFile(filename);
  AddShader(shader);
}

void GLShaderProgram::AddShaderFromSource(ShaderType shader_type,
                                          const string &shader_source) {
  shared_ptr<GLShader> shader(make_shared<GLShader>(shader_type));
  shader->CompileFromSource(shader_source);
  AddShader(shader);
}

void GLShaderProgram::AddShader(shared_ptr<GLShader> shader) {
  glAttachShader(program_id_, shader->get_shader_id());
  GLCheckError();

  shaders_.push_back(shader);
}

void GLShaderProgram::Release() {
  if (program_id_ == -1) {
    return;
  }
  GLCheckError();
  glDeleteProgram(program_id_);
  program_id_ = -1;
  GLCheckError();
  shaders_.clear();
}

void GLShaderProgram::Bind(bool bind_it) {
  is_binded_ = false;
  if (!bind_it) {
    glUseProgram(0);
    GLCheckError();
    return;
  }

  bool all_success = true;
  bool link_dirty = false;
  for (auto ls_iter = shaders_.begin(); ls_iter != shaders_.end(); ++ls_iter) {
    shared_ptr<GLShader> shader = *ls_iter;
    const GLShader::RecompileStatus recomp(shader->Recompile());
    if (recomp.recompiled) {
      link_dirty = true;
    }
    all_success = all_success && recomp.success;
  }

  if (!all_success) return;

  if (link_dirty) {
    Link();
  }

  if (!is_linked_) return;

  glUseProgram(program_id_);
  GLCheckError();

  is_binded_ = bind_it;
}

bool GLShaderProgram::Link() {
  glLinkProgram(program_id_);
  GLCheckError();

  GLint link_status = GL_FALSE;
  glGetProgramiv(program_id_, GL_LINK_STATUS, &link_status);
  GLCheckError();

  last_link_log_.clear();

  if (link_status == GL_TRUE) {
    WriteLog("");
    is_linked_ = true;
    return true;
  }

  GLint log_len;
  glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &log_len);
  GLCheckError();
  last_link_log_.resize(log_len, '\0');

  glGetProgramInfoLog(program_id_, log_len, NULL, &last_link_log_[0]);
  GLCheckError();
  WriteLog(last_link_log_);

  is_linked_ = false;
  return false;
}

void GLShaderProgram::WriteLog(const string &log) {
  if (shaders_.empty()) {
    return;
  }

  string fname = shaders_[0]->get_shader_filename() + ".link";
  ofstream output(fname.c_str());
  output << log;
  output.close();
}

bool GLShaderProgram::HasUniform(const std::string &name) const {
  if (!is_linked_) return -1;
  return glGetUniformLocation(program_id_, name.c_str()) > -1;
}

bool GLShaderProgram::HasAttrib(const std::string &name) const {
  if (!is_linked_) return -1;
  int loc = glGetAttribLocation(program_id_, name.c_str());
  return loc > -1;
}

GLint GLShaderProgram::GetUniformLocation(const string &name) {
  if (!is_linked_) return -1;
  const GLint loc = glGetUniformLocation(program_id_, name.c_str());
  GLCheckError();
  if (loc < 0) {
    if (not_found_variables_.count(name) == 0) {
      not_found_variables_.insert(name);
      cerr << "Vertex uniform " << name << " not found" << endl;
    }
  }
  return loc;
}

GLint GLShaderProgram::GetAttribLocation(const string &name) {
  if (!is_linked_) return -1;
  const GLint loc = glGetAttribLocation(program_id_, name.c_str());
  GLCheckError();
  if (loc < 0) {
    if (not_found_variables_.count(name) == 0) {
      cerr << "Vertex attribute " << name << " not found" << endl;
      not_found_variables_.insert(name);
    }
  }

  return loc;
}

namespace {
template <typename Scalar>
inline void SetUniform1(int loc, torch::TensorAccessor<Scalar, 1> acs) {
  assert(false);
}

template <>
inline void SetUniform1<float>(int loc, torch::TensorAccessor<float, 1> acs) {
  glUniform1f(loc, acs[0]);
}

template <>
inline void SetUniform1<int>(int loc, torch::TensorAccessor<int, 1> acs) {
  glUniform1i(loc, acs[0]);
}

template <typename Scalar>
inline void SetUniform2(int loc, torch::TensorAccessor<Scalar, 1> acs) {
  assert(false);
}

template <>
inline void SetUniform2<float>(int loc, torch::TensorAccessor<float, 1> acs) {
  glUniform2f(loc, acs[0], acs[1]);
  GLCheckError();
}

template <>
inline void SetUniform2<int>(int loc, torch::TensorAccessor<int, 1> acs) {
  glUniform2i(loc, acs[0], acs[1]);
  GLCheckError();
}

template <typename Scalar>
inline void SetUniform3(int loc, torch::TensorAccessor<Scalar, 1> acs) {
  assert(false);
}

template <>
inline void SetUniform3<float>(int loc, torch::TensorAccessor<float, 1> acs) {
  glUniform3f(loc, acs[0], acs[1], acs[2]);
  GLCheckError();
}

template <>
inline void SetUniform3<int>(int loc, torch::TensorAccessor<int, 1> acs) {
  glUniform3i(loc, acs[0], acs[1], acs[2]);
  GLCheckError();
}

template <typename Scalar>
inline void SetUniform4(int loc, torch::TensorAccessor<Scalar, 1> acs) {
  assert(false);
}

template <>
inline void SetUniform4<float>(int loc, torch::TensorAccessor<float, 1> acs) {
  glUniform4f(loc, acs[0], acs[1], acs[2], acs[3]);
  GLCheckError();
}

template <>
inline void SetUniform4<int>(int loc, torch::TensorAccessor<int, 1> acs) {
  glUniform4i(loc, acs[0], acs[1], acs[2], acs[3]);
  GLCheckError();
}

}  // namespace

void GLShaderProgram::SetUniform(const string &name,
                                 const torch::Tensor &tensor) {
  if (!is_binded_) return;

  if (tensor.is_cuda()) {
    stringstream msg;
    msg << "Only CPU tensors can set uniforms: " << name;
    throw Error(msg);
  }

  const GLint location = GetUniformLocation(name);
  const int ndims = tensor.ndimension();

  if (ndims == 1) {
    switch (tensor.scalar_type()) {
      case torch::kFloat:
      case torch::kInt32:
        break;
      default:
        throw Error(
            "Only float32 or int32 tensors can set scalar or vector uniforms");
    }

    AT_DISPATCH_ALL_TYPES(tensor.scalar_type(), "Uniform", ([&] {
                            const auto accessor =
                                tensor.accessor<scalar_t, 1>();
                            switch (tensor.size(0)) {
                              case 1:
                                SetUniform1<scalar_t>(location, accessor);
                                break;
                              case 2:
                                SetUniform2<scalar_t>(location, accessor);
                                break;
                              case 3:
                                SetUniform3<scalar_t>(location, accessor);
                                break;
                              case 4:
                              default:
                                SetUniform4<scalar_t>(location, accessor);
                            }
                          }));

    if (glGetError() != GL_NO_ERROR) {
      stringstream err;
      err << "Uniform " << name
          << ": cannot set value. Does the type matches the one in shader?";
      throw Error(err);
    }
  } else if (ndims == 2) {
    const int rows = tensor.size(0);
    const int cols = tensor.size(1);

    if (tensor.scalar_type() != torch::kFloat) {
      throw Error("Only float32 tensors can set matrix uniforms");
    }

    auto strides = tensor.strides();
    // OpenGL matrices are ordered column-wise in memory.
    GLenum transpose = GL_TRUE;
    if (strides[0] < strides[1]) transpose = GL_FALSE;

    if (rows == 4 && cols == 4) {
      glUniformMatrix4fv(location, 1, transpose, tensor.data_ptr<float>());
      GLCheckError();
    } else if (rows == 3 && cols == 3) {
      glUniformMatrix3fv(location, 1, transpose, tensor.data_ptr<float>());
      GLCheckError();
    } else if (rows == 2 && cols == 2) {
      glUniformMatrix2fv(location, 1, transpose, tensor.data_ptr<float>());
      GLCheckError();
    } else {
      throw Error(
          "Can't set uniform matrix using non square matrix or larger than 4.");
    }
    GLCheckError();
  } else {
    throw Error("Can't set uniform using a tensor larger than 2-dims");
  }
}

}  // namespace tenviz
