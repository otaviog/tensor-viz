#pragma once

#include <memory>
#include <string>

#include "gl_common.hpp"

#include <cuda_gl_interop.h>
#include <torch/torch.h>

#include "context_resource.hpp"
#include "cuda_memory.hpp"
#include "dim.hpp"
#include "dtype.hpp"
#include "error.hpp"
#include "gl_error.hpp"

namespace tenviz {

/**
 * Supported GL texture parameters by TensorViz.
 */
class GLTextureParameters {
 public:
  static GLTextureParameters GoodQuality() {
    return GLTextureParameters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT,
                               GL_REPEAT, false);
  }

  static GLTextureParameters MediumQuality() {
    return GLTextureParameters(GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, GL_REPEAT,
                               GL_REPEAT, true);
  }

  static GLTextureParameters PoorQuality() {
    return GLTextureParameters(GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT,
                               true);
  }

  static GLTextureParameters RectGood() {
    return GLTextureParameters(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER,
                               GL_CLAMP_TO_BORDER);
  }

  GLTextureParameters() { *this = GoodQuality(); }

  GLTextureParameters(GLenum minFilter, GLenum magFilter, GLenum wrapS,
                      GLenum wrapT, bool anisotropic = true) {
    this->minFilter = minFilter;
    this->magFilter = magFilter;
    this->wrapS = wrapS;
    this->wrapT = wrapT;
    this->useAnisotropic = anisotropic;
  }

  GLenum magFilter, minFilter, wrapS, wrapT;
  bool useAnisotropic, shadowMap;
};

enum TexTarget {
  k1D = GL_TEXTURE_1D,
  k2D = GL_TEXTURE_2D,
  k3D = GL_TEXTURE_3D,
  kRectangle = GL_TEXTURE_RECTANGLE
};

class GLTexture : public IContextResource {
 public:
  static const GLTextureParameters kTexDefaultParms;

  static std::shared_ptr<GLTexture> CreateFromTensor(torch::Tensor image,
                                                     TexTarget target = k2D);

  static std::shared_ptr<GLTexture> CreateEmpty(int width, int height,
                                                int depth, int channels,
                                                DType type,
                                                TexTarget target = k2D);

  static std::shared_ptr<GLTexture> Create(TexTarget target);

  static void RegisterPybind(pybind11::module &m,
                             IContextResource::PythonClassDef &base_class);

  GLTexture(GLenum target);

  GLTexture(const GLTexture &cpy) = delete;

  GLTexture &operator=(const GLTexture &cpy) = delete;

  ~GLTexture();

  void Release() override;

  GLuint get_id() const { return tex_; }

  void SetParameters(const GLTextureParameters &parms);

  const GLTextureParameters &get_parameters() const { return parms_; }

  void FromTensor(torch::Tensor image);

  torch::Tensor ToTensor(bool keep_device = true, bool non_blocking = false);

  void Empty(const std::vector<long> &dim_sizes, DType btype);

  void TexImage(GLenum internal_format, int width, int height, GLenum format,
                GLenum type, const void *data);

  void TexImage3D(GLenum internal_format, int width, int height, int depth,
                  GLenum format, GLenum type, const void *data);

  void Bind(bool bind, int texture_unit = -1) const;

  int get_width() const { return dim_.get_width(); }

  int get_height() const { return dim_.get_height(); }

  int get_depth() const { return dim_.get_depth(); }

  // Dim3 get_dim() const { return dim_; }

  TexTarget get_target() const { return static_cast<TexTarget>(target_); }

  DType get_type() const { return cast_type<DType>(type_); }

 private:
  torch::Tensor ToTensorCUDA(bool keep_on_device, bool non_blocking);

  torch::Tensor ToTensorGL();

  GLuint tex_;
  cudaGraphicsResource_t cuda_resource_;
  GLenum target_, format_, type_;
  GLTextureParameters parms_;
  Dim3 dim_;

  torch::Tensor tex_tensor_;
};
}  // namespace tenviz
