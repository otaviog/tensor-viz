#include "draw_program.hpp"

#include "gl_buffer.hpp"
#include "gl_error.hpp"
#include "gl_shader_program.hpp"
#include "gl_texture.hpp"
#include "math.hpp"
#include "scoped_bind.hpp"

using namespace std;

namespace tenviz {
void DrawProgram::RegisterPybind(
    py::module &m, pybind11::class_<ANode, std::shared_ptr<ANode>> &anode) {
  py::enum_<DrawMode>(m, "DrawMode")
      .value("Triangles", DrawMode::kTriangles)
      .value("TriangleStrip", DrawMode::kTrianglesStrip)
      .value("Quads", DrawMode::kQuads)
      .value("Points", DrawMode::kPoints)
      .value("Lines", DrawMode::kLines)
      .export_values();

  py::enum_<MatPlaceholder>(m, "MatPlaceholder")
      .value("Modelview", MatPlaceholder::kModelview)
      .value("Projection", MatPlaceholder::kProjection)
      .value("ProjectionModelview", MatPlaceholder::kProjectionModelview)
      .value("NormalModelview", MatPlaceholder::kNormalModelview)
      .value("Object", MatPlaceholder::kObject)
      .export_values();

  py::class_<DrawProgram, shared_ptr<DrawProgram>>(m, "DrawProgram", anode)
      .def(py::init<DrawMode, shared_ptr<GLShaderProgram>, bool>())
      .def("set_bounds", &DrawProgram::SetBounds)
      .def("__setitem__",
           py::overload_cast<const string &, shared_ptr<GLBuffer>>(
               &DrawProgram::SetItem))
      .def("__setitem__",
           py::overload_cast<const string &, const torch::Tensor &>(
               &DrawProgram::SetItem))
      .def("__setitem__", py::overload_cast<const string &, MatPlaceholder>(
                              &DrawProgram::SetItem))
      .def("__setitem__",
           py::overload_cast<const string &, shared_ptr<GLTexture>>(
               &DrawProgram::SetItem))
      .def("__setitem__",
           py::overload_cast<const string &, int>(&DrawProgram::SetItem))
      .def("__setitem__",
           py::overload_cast<const string &, float>(&DrawProgram::SetItem))
      .def("__getitem__", &DrawProgram::GetItem)
      .def_readwrite("indices", &DrawProgram::indices)
      .def_readwrite("style", &DrawProgram::style);
}

DrawProgram::DrawProgram(DrawMode mode,
                         std::shared_ptr<GLShaderProgram> program,
                         bool ignore_missing)
    : bounds_(Bounds::UnitBounds()) {
  draw_mode_ = mode;
  program_ = program;
  ignore_missing_ = ignore_missing;

  indices = GLBuffer::Create(BufferTarget::kElement, BufferUsage::kDynamic);
  max_draw_elems_ = -1;
  program->Bind(true);
  program->Bind(false);

  glGenVertexArrays(1, &vao_);
  GLCheckError();
}

void DrawProgram::Draw(const Eigen::Matrix4f &projection,
                       const Eigen::Matrix4f &view) {
  Eigen::Matrix4f modelview = view * transform;
  Eigen::Matrix4f proj_modelview = projection * modelview;
  Eigen::Matrix3f normal_modelview = math::ComputeNormalModelview(modelview);

  ScopedBind<GLShaderProgram> program_bind(program_);
  for (const auto &key_pholder : matrix_placeholders_) {
    const auto &key = key_pholder.first;
    const auto &pholder = key_pholder.second;

    switch (pholder) {
      case MatPlaceholder::kModelview:
        program_->SetUniformValue(key, modelview);
        break;
      case MatPlaceholder::kProjection:
        program_->SetUniformValue(key, projection);
        break;
      case MatPlaceholder::kProjectionModelview:
        program_->SetUniformValue(key, proj_modelview);
        break;
      case MatPlaceholder::kNormalModelview:
        program_->SetUniformValue(key, normal_modelview);
        break;
      case MatPlaceholder::kObject:
        program_->SetUniformValue(key, transform);
        break;
    }
  }

  set<int> enabled_attribs;
  int vertex_size = -1;
  GLCheckError();
  glBindVertexArray(vao_);
  GLCheckError();
  for (const auto &key_buffer : buffers_) {
    const auto &key = key_buffer.first;
    auto &buffer = key_buffer.second;
    const auto attrib_loc = program_->GetAttribLocation(key);

    if (vertex_size > -1 && buffer->get_size(0) != vertex_size) {
      throw Error("Buffers vertices size doesn't match");
    }

    if (attrib_loc < 0) {
      glGetError();
      continue;
    }

    vertex_size = buffer->get_size(0);

    ScopedBind<GLBuffer> buffer_bind(buffer);
    glEnableVertexAttribArray(attrib_loc);
    GLCheckError();

    enabled_attribs.insert(attrib_loc);

    int channels = 1;
    if (buffer->get_size().size() > 1) {
      channels = buffer->get_size(1);
    }

    const GLenum gl_type = buffer->get_gl_type();
    if (buffer->integer_attrib || gl_type == GL_INT ||
        gl_type == GL_UNSIGNED_INT) {
      glVertexAttribIPointer(attrib_loc, channels, buffer->get_gl_type(), 0,
                             nullptr);
    } else {
      const GLenum normalize = buffer->normalize ? GL_TRUE : GL_FALSE;
      glVertexAttribPointer(attrib_loc, channels, buffer->get_gl_type(),
                            normalize, 0, nullptr);
    }
    GLCheckError();
  }

  auto disable_all_attribs = [&] {
    for (auto attrib_loc : enabled_attribs) {
      glDisableVertexAttribArray(attrib_loc);
      GLCheckError();
    }
  };

  if (vertex_size == -1) {
    disable_all_attribs();
    return;
  }

  int tex_unit = 0;
  for (auto name_tex : textures_) {
    const string &name = name_tex.first;
    shared_ptr<GLTexture> tex = name_tex.second;
    tex->Bind(true, tex_unit);
    program_->SetUniformValue(name, tex_unit);
    ++tex_unit;
  }

  for (auto key_value : uniforms_) {
    const auto &key = key_value.first;
    const auto &value = key_value.second;

    program_->SetUniform(key, value);
  }

  const GLenum draw_mode = static_cast<GLenum>(draw_mode_);
  Style::Scoped style_scop(style);
  if (!indices->is_empty()) {
    ScopedBind<GLBuffer> ind_bind(indices);
    GLenum type;
    switch (indices->get_gl_type()) {
      case GL_UNSIGNED_INT:
      case GL_UNSIGNED_SHORT:
      case GL_UNSIGNED_BYTE:
        type = indices->get_gl_type();
        break;
      case GL_INT:
        type = GL_UNSIGNED_INT;
        break;
      case GL_SHORT:
        type = GL_UNSIGNED_SHORT;
        break;
      case GL_BYTE:
        type = GL_UNSIGNED_BYTE;
        break;
      default:
        throw Error("Indice buffer must be integers.");
    }
    size_t size = indices->get_size(0);
    if (max_draw_elems_ > 0) {
      size = min(size, size_t(max_draw_elems_));
    }
    if (indices->get_dim() == 2) {
      size *= indices->get_size(1);
    }

    glDrawElements(draw_mode, size, type, nullptr);
    GLCheckError();
  } else {
    glDrawArrays(draw_mode, 0, vertex_size);
    GLCheckError();
  }
  glBindVertexArray(0);
  GLCheckError();

  disable_all_attribs();

  tex_unit = 0;
  for (auto name_tex : textures_) {
    shared_ptr<GLTexture> tex = name_tex.second;
    tex->Bind(false, tex_unit);
  }
}

shared_ptr<DrawProgram> DrawProgram::Clone() {
  DrawProgram *new_program =
      new DrawProgram(draw_mode_, program_, ignore_missing_);

  new_program->buffers_ = buffers_;
  new_program->matrix_placeholders_ = matrix_placeholders_;
  new_program->uniforms_ = uniforms_;
  new_program->textures_ = textures_;

  return shared_ptr<DrawProgram>(new_program);
}

void DrawProgram::SetBounds(const torch::Tensor &points) {
  bounds_ = Bounds::FromPoints(points);
}

void DrawProgram::SetItem(const std::string &name,
                          std::shared_ptr<GLBuffer> buffer) {
  if (!ignore_missing_ && !program_->HasAttrib(name)) {
    stringstream format;
    format << "Program parameter `" << name << "` not found";
    throw Error(format);
  }
  if (buffer != nullptr) {
    buffers_[name] = buffer;
  } else {
    buffers_.erase(name);
  }
}

void DrawProgram::SetItem(const std::string &name, MatPlaceholder placeholder) {
  if (!ignore_missing_ && !program_->HasUniform(name)) {
    stringstream format;
    format << "Program parameter `" << name << "` not found";
    throw Error(format);
  }
  matrix_placeholders_[name] = placeholder;
}

void DrawProgram::SetItem(const std::string &name,
                          const torch::Tensor &tensor) {
  if (program_->HasAttrib(name)) {
    switch (tensor.scalar_type()) {
      case torch::kDouble:
        throw Error("Double tensors can't be assigned to GL buffers");
        break;
      default:
        break;
    }
    if (!buffers_.count(name)) {
      buffers_[name] = GLBuffer::Create();
    }

    buffers_[name]->FromTensor(tensor);
  } else if (program_->HasUniform(name)) {
    uniforms_[name] = tensor;
  } else if (!ignore_missing_) {
    stringstream format;
    format << "Program parameter `" << name << "` not found";
    throw Error(format);
  }
}

void DrawProgram::SetItem(const string &name, shared_ptr<GLTexture> texture) {
  if (!ignore_missing_ && !program_->HasUniform(name)) {
    stringstream format;
    format << "Program parameter `" << name << "` not found";
    throw Error(format);
  }

  if (texture != nullptr) {
    textures_[name] = texture;
  } else {
    textures_.erase(name);
  }
}

void DrawProgram::SetItem(const std::string &name, float value) {
  if (!ignore_missing_ && !program_->HasUniform(name)) {
    stringstream format;
    format << "Program parameter `" << name << "` not found";
    throw Error(format);
  }

  uniforms_[name] = torch::full({1}, value, torch::kFloat);
}

void DrawProgram::SetItem(const std::string &name, int value) {
  if (!ignore_missing_ && !program_->HasUniform(name)) {
    stringstream format;
    format << "Program parameter `" << name << "` not found";
    throw Error(format);
  }

  uniforms_[name] = torch::full({1}, value, torch::kInt32);
}

pybind11::object DrawProgram::GetItem(const string &name) {
  return py::cast(program_);
}

}  // namespace tenviz
