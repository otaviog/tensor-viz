#include "gl_texture.hpp"

#include <cassert>

#include <cuda.h>
#include <cuda_runtime.h>
#include <stdexcept>

#include "cuda_error.hpp"
#include "dtype.hpp"

using namespace std;

namespace tenviz {
namespace {
const GLuint GL_SENTINEL = numeric_limits<GLuint>::max();
}

inline bool IsFomartCudaCompatible(GLenum format) {
  static std::set<GLenum> allowed_formats(
      {GL_RED, GL_RG, GL_RGB, GL_RGBA, GL_LUMINANCE, GL_ALPHA,
       GL_LUMINANCE_ALPHA, GL_INTENSITY,
       // Second bullet
       GL_R8, GL_RG8, GL_RGBA8, GL_R16, GL_RG16, GL_RGBA16, GL_R16F, GL_RG16F,
       GL_RGBA16F, GL_R32F, GL_RG32F, GL_RGBA32F, GL_R8UI, GL_RG8UI, GL_RGBA8UI,
       GL_R16UI, GL_RG16UI, GL_RGBA16UI, GL_R32UI, GL_RG32UI, GL_RGBA32UI,
       GL_R8I, GL_RG8I, GL_RGBA8I, GL_R16I, GL_RG16I, GL_RGBA16I, GL_R32I,
       GL_RG32I, GL_RGBA32I,
       // Third bullet
       GL_LUMINANCE8, GL_ALPHA8, GL_INTENSITY8, GL_LUMINANCE16, GL_ALPHA16,
       GL_INTENSITY16, GL_LUMINANCE16F_ARB, GL_ALPHA16F_ARB,
       GL_INTENSITY16F_ARB, GL_LUMINANCE32F_ARB, GL_ALPHA32F_ARB,
       GL_INTENSITY32F_ARB, GL_LUMINANCE8UI_EXT, GL_ALPHA8UI_EXT,
       GL_INTENSITY8UI_EXT, GL_LUMINANCE16UI_EXT, GL_ALPHA16UI_EXT,
       GL_INTENSITY16UI_EXT, GL_LUMINANCE32UI_EXT, GL_ALPHA32UI_EXT,
       GL_INTENSITY32UI_EXT, GL_LUMINANCE8I_EXT, GL_ALPHA8I_EXT,
       GL_INTENSITY8I_EXT, GL_LUMINANCE16I_EXT, GL_ALPHA16I_EXT,
       GL_INTENSITY16I_EXT, GL_LUMINANCE32I_EXT, GL_ALPHA32I_EXT,
       GL_INTENSITY32I_EXT,
       // RGB extra
       GL_RGB8, GL_RGB8I, GL_RGB8UI, GL_RGB32F, GL_RGB32I, GL_RGB32UI});

  return allowed_formats.count(format) > 0;
}

inline GLenum GetGLInternalFormat(int channels, torch::ScalarType dtype) {
  switch (channels) {
    case 1:
      switch (dtype) {
        case torch::kFloat:
          return GL_R32F;
        case torch::kInt32:
          return GL_LUMINANCE32I_EXT;
        case torch::kInt16:
          return GL_R16I;
        default:
          return GL_RED;
      }
    case 3:
      switch (dtype) {
        case torch::kFloat:
          return GL_RGB32F;
        case torch::kInt32:
          return GL_RGB32I;
        default:
          return GL_RGB;
      }
    case 4:
      switch (dtype) {
        case torch::kFloat:
          return GL_RGBA32F;
        case torch::kInt32:
          return GL_RGBA32I;
        default:
          return GL_RGBA;
      }
    default:
      throw Error("Not possible to convert tensor to texture format");
  }
}

inline GLenum GetGLFormat(int channels, torch::ScalarType dtype) {
  switch (channels) {
    case 1:
      switch (dtype) {
        case torch::kInt32:
          return GL_LUMINANCE_INTEGER_EXT;
        case torch::kInt8:
        case torch::kUInt8:
        case torch::kFloat:
        default:
          return GL_RED;
      }
    case 3:
      switch (dtype) {
        case torch::kInt32:
          return GL_RGB_INTEGER;
        case torch::kInt8:
        case torch::kUInt8:
        case torch::kFloat:
        default:
          return GL_RGB;
      }
    case 4:
      switch (dtype) {
        case torch::kInt32:
          return GL_RGBA_INTEGER;
        case torch::kInt8:
        case torch::kUInt8:
        case torch::kFloat:
        default:
          return GL_RGBA;
      }
    default:
      throw Error("Unknown tensor image format");
  }
}

const GLTextureParameters GLTexture::kTexDefaultParms = GLTextureParameters();

shared_ptr<GLTexture> GLTexture::Create(TexTarget target) {
  auto tex = make_shared<GLTexture>(static_cast<GLenum>(target));

  IContextResource::RegisterResourceOnCurrent(tex);
  return tex;
}

void GLTexture::Empty(const std::vector<long> &dim_sizes, DType btype) {
  if (dim_sizes.size() < 2) {
    throw Error("Invalid size for texture");
  }

  const int width = dim_sizes[1];
  const int height = dim_sizes[0];

  const GLenum gl_type = cast_type<GLenum>(btype);
  const torch::ScalarType dtype = cast_type<torch::ScalarType>(gl_type);

  switch (target_) {
    case GL_TEXTURE_RECTANGLE:
    case GL_TEXTURE_2D: {
      int depth = 1;
      if (dim_sizes.size() > 2) {
        depth = dim_sizes[2];
      }

      TexImage(GetGLInternalFormat(depth, dtype), width, height,
               GetGLFormat(depth, dtype), gl_type, nullptr);
    } break;

    case GL_TEXTURE_3D: {
      if (dim_sizes.size() < 3) {
        throw Error("Invalid size of 3D texture");
      }

      int channels = 1;
      if (dim_sizes.size() > 3) {
        channels = dim_sizes[3];
      }
      TexImage3D(GetGLInternalFormat(channels, dtype), width, height,
                 dim_sizes[2], GetGLFormat(channels, dtype), gl_type, nullptr);
    } break;
    default:
      throw Error("Unsupported texture format");
  }
}

void GLTexture::RegisterPybind(pybind11::module &m,
                               IContextResource::PythonClassDef &base_class) {
  pybind11::class_<GLTexture, shared_ptr<GLTexture>>(m, "Texture", base_class)
      .def(py::init<TexTarget>())
      .def("to_tensor", &GLTexture::ToTensor, py::arg("keep_device") = true,
           py::arg("non_blocking") = false)
      .def("from_tensor", &GLTexture::FromTensor)
      .def_property("width", &GLTexture::get_width, nullptr)
      .def_property("height", &GLTexture::get_height, nullptr)
      .def_property("depth", &GLTexture::get_depth, nullptr)
      .def_property("target", &GLTexture::get_target, nullptr)
      .def_property("dtype", &GLTexture::get_type, nullptr);

  pybind11::enum_<TexTarget>(m, "TexTarget")
      .value("k1D", TexTarget::k1D)
      .value("k2D", TexTarget::k2D)
      .value("k3D", TexTarget::k3D)
      .value("Rectangle", TexTarget::kRectangle)
      .export_values();
}

GLTexture::GLTexture(GLenum target) : target_(target) {
  if (target_ == GL_TEXTURE_RECTANGLE)
    parms_ = GLTextureParameters::RectGood();
  else
    parms_ = GLTextureParameters::GoodQuality();

  glGenTextures(1, &tex_);
  GLCheckError();

  cuda_resource_ = nullptr;

  format_ = GL_NONE;
  type_ = GL_NONE;
}

GLTexture::~GLTexture() { Release(); }

void GLTexture::Release() {
  if (cuda_resource_ != nullptr) {
    CudaSafeCall(cudaGraphicsUnregisterResource(cuda_resource_));
    cuda_resource_ = nullptr;
  }

  if (tex_ != GL_SENTINEL) {
    glDeleteTextures(1, &tex_);
    tex_ = GL_SENTINEL;
  }
  GLCheckError();
}

void GLTexture::SetParameters(const GLTextureParameters &parms) {
  Bind(true);
  parms_ = parms;
  glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, parms.minFilter);
  GLCheckError();

  glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, parms.magFilter);
  GLCheckError();

  glTexParameteri(target_, GL_TEXTURE_WRAP_S, parms.wrapS);
  GLCheckError();

  glTexParameteri(target_, GL_TEXTURE_WRAP_T, parms.wrapT);
  GLCheckError();

  if (parms.shadowMap) {
    const float borderColor[] = {1.0f, 0.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    GLCheckError();

    glTexParameteri(target_, GL_TEXTURE_COMPARE_MODE_ARB,
                    GL_COMPARE_R_TO_TEXTURE);
    GLCheckError();

    glTexParameteri(target_, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
    GLCheckError();

    glTexParameteri(target_, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY);
    GLCheckError();
  }

  Bind(false);
}

void GLTexture::Bind(bool bind, int texture_unit) const {
  if (texture_unit > -1) {
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    GLCheckError();
  }

  if (bind) {
    glEnable(target_);
    GLCheckError();

    glBindTexture(target_, tex_);
    GLCheckError();
  } else {
    glBindTexture(target_, 0);
    GLCheckError();
  }
}

void GLTexture::TexImage(GLenum internal_format, int width, int height,
                         GLenum format, GLenum type, const void *data) {
  if (cuda_resource_ != nullptr) {
    CudaSafeCall(cudaGraphicsUnregisterResource(cuda_resource_));
    cuda_resource_ = nullptr;
  }

  Bind(true);
  if (target_ == GL_TEXTURE_1D) {
    glTexImage1D(target_, 0, internal_format, width, 0, format, type, data);
    GLCheckError();
  } else if (target_ == GL_TEXTURE_2D) {
    glTexImage2D(target_, 0, internal_format, width, height, 0, format, type,
                 data);
    GLCheckError();

    CudaSafeCall(cudaGraphicsGLRegisterImage(&cuda_resource_, tex_, target_,
                                             cudaGraphicsMapFlagsNone));
    glGenerateMipmap(GL_TEXTURE_2D);
    GLCheckError();
  } else {
    if (dim_ != Dim2(width, height)) {
      glTexImage2D(target_, 0, internal_format, width, height, 0, format, type,
                   data);
      GLCheckError();
    } else {
      glTexSubImage2D(target_, 0, 0, 0, width, height, format, type, data);
      GLCheckError();
    }
  }

  if (IsFomartCudaCompatible(internal_format)) {
    CudaSafeCall(cudaGraphicsGLRegisterImage(&cuda_resource_, tex_, target_,
                                             cudaGraphicsMapFlagsNone));
  }

  SetParameters(parms_);
  dim_ = Dim3(width, height, 1);
  format_ = format;
  type_ = type;
  Bind(false);
}

void GLTexture::TexImage3D(GLenum internal_format, int width, int height,
                           int depth, GLenum format, GLenum type,
                           const void *data) {
  Bind(true);
  glTexImage3D(target_, 0, internal_format, width, height, depth, 0, format,
               type, data);
  GLCheckError();
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
  GLCheckError();
  // SetParameters(parms_);
  dim_ = Dim3(width, height, depth);
  format_ = format;
  type_ = type;
  Bind(false);
}

namespace {
int guess_image1d_channels(const torch::IntArrayRef &dims) {
  if (dims.size() == 1) {
    return 1;
  } else if (dims.size() == 2) {
    return dims[1];
  }
  assert(false);
  return -1;
}

int guess_image2d_channels(const torch::IntArrayRef &dims) {
  if (dims.size() == 2) {
    return 1;
  } else if (dims.size() == 3) {
    return dims[2];
  }
  assert(false);
  return -1;
}

int guess_image3d_channels(const torch::IntArrayRef &dims) {
  if (dims.size() == 3) {
    return 1;
  } else if (dims.size() == 4) {
    return dims[3];
  }
  assert(false);
  return -1;
}
}  // namespace

void GLTexture::FromTensor(torch::Tensor image) {
  if (!image.is_contiguous()) {
    image = image.contiguous();
  }
  GLenum internal_format = GL_NONE;

  if (target_ == GL_TEXTURE_1D) {
    const int channels = guess_image2d_channels(image.sizes());
    const GLenum gl_format = GetGLFormat(channels, image.scalar_type());
    if (internal_format == GL_NONE) {
      internal_format = GetGLInternalFormat(channels, image.scalar_type());
    }
    TexImage(internal_format, image.size(0), 1, gl_format,
             cast_type<GLenum>(image.scalar_type()), image.data_ptr());
  } else if (target_ == GL_TEXTURE_2D || target_ == GL_TEXTURE_RECTANGLE) {
    const int channels = guess_image2d_channels(image.sizes());
    const GLenum gl_format = GetGLFormat(channels, image.scalar_type());
    if (internal_format == GL_NONE) {
      internal_format = GetGLInternalFormat(channels, image.scalar_type());
    }

    TexImage(internal_format, image.size(1), image.size(0), gl_format,
             cast_type<GLenum>(image.scalar_type()), image.data_ptr());
  } else if (target_ == GL_TEXTURE_3D) {
    const int channels = guess_image3d_channels(image.sizes());
    const GLenum gl_format = GetGLFormat(channels, image.scalar_type());
    if (internal_format == GL_NONE) {
      internal_format = GetGLInternalFormat(channels, image.scalar_type());
    }

    TexImage3D(internal_format, image.size(1), image.size(0), image.size(2),
               gl_format, cast_type<GLenum>(image.scalar_type()),
               image.data_ptr());
  }
}

namespace {
inline int GLformatToDepth(GLenum format) {
  switch (format) {
    case GL_RGBA:
    case GL_RGBA8:
    case GL_RGBA32F:
    case GL_RGBA_INTEGER:
      return 4;
    case GL_RGB:
    case GL_RGB8:
    case GL_RGB32F:
    case GL_RGB_INTEGER:
      return 3;
    case GL_LUMINANCE_INTEGER_EXT:
    case GL_RED:
    case GL_RED_INTEGER:
    case GL_DEPTH_COMPONENT:
      return 1;
    default:
      throw Error("Unknown texture format");
  }
}
}  // namespace
template <>
torch::ScalarType cast_type(CUarray_format format) {
  switch (format) {
    case CU_AD_FORMAT_UNSIGNED_INT8:
      return torch::kUInt8;
      break;
    case CU_AD_FORMAT_UNSIGNED_INT16:
      throw Error("uint16 scalars have no counterpart on torch");
      break;
    case CU_AD_FORMAT_UNSIGNED_INT32:
      throw Error("uint32 scalars have no counterpart on torch");
      break;
    case CU_AD_FORMAT_SIGNED_INT8:
      return torch::kInt8;
      break;
    case CU_AD_FORMAT_SIGNED_INT16:
      return torch::kInt16;
      break;
    case CU_AD_FORMAT_SIGNED_INT32:
      return torch::kInt32;
      break;
    case CU_AD_FORMAT_HALF:
      return torch::kHalf;
      break;
    case CU_AD_FORMAT_FLOAT:
      return torch::kFloat;
      break;
    default:
      throw Error("Unsupported CUarray format");
  }
}

torch::Tensor GLTexture::ToTensor(bool keep_on_device, bool non_blocking) {
  if (cuda_resource_ != nullptr) {
    return ToTensorCUDA(keep_on_device, non_blocking);
  } else {
    return ToTensorGL();
  }
}

namespace {
bool AreSizesEqual(const std::vector<int64_t> &lfs, torch::IntArrayRef rhs) {
  if (lfs.size() != rhs.size()) return false;

  for (int i = 0; i < lfs.size(); ++i) {
    if (lfs[i] != rhs[i]) {
      return false;
    }
  }

  return true;
}

void EnsureTensorSize(torch::Tensor &tensor,
                      const std::vector<int64_t> &dim_sizes,
                      const std::vector<int64_t> &stride_sizes,
                      const torch::TensorOptions &opts) {
  if (tensor.numel() == 0 || !AreSizesEqual(dim_sizes, tensor.sizes()) ||
      !AreSizesEqual(stride_sizes, tensor.strides())) {
    tensor = torch::empty_strided(dim_sizes, stride_sizes, opts);
  }
}
}  // namespace

torch::Tensor GLTexture::ToTensorCUDA(bool keep_on_device, bool non_blocking) {
  // This method doesn't support 3D texture

  assert(target_ != GL_TEXTURE_3D);
  CudaSafeCall(cudaGraphicsMapResources(1, &cuda_resource_));

  cudaArray *tex_array;
  CudaSafeCall(
      cudaGraphicsSubResourceGetMappedArray(&tex_array, cuda_resource_, 0, 0));

  CUDA_ARRAY_DESCRIPTOR desc;
  cuArrayGetDescriptor(&desc, reinterpret_cast<CUarray>(tex_array));

  torch::ScalarType dtype = cast_type<torch::ScalarType>(desc.Format);
  const long type_size = GetTypeSize(dtype);
  const int depth = GLformatToDepth(format_);
  std::vector<int64_t> dim_sizes = {long(desc.Height), long(desc.Width), depth};
  std::vector<int64_t> stride_sizes = {long(desc.Width * desc.NumChannels),
                                       long(desc.NumChannels), 1};

  auto tensor_opts = torch::TensorOptions().dtype(dtype);
  cudaMemcpyKind copy_kind = cudaMemcpyDeviceToHost;

  if (keep_on_device) {
    tensor_opts = tensor_opts.device(torch::kCUDA, 0);
    copy_kind = cudaMemcpyDeviceToDevice;
  }
  EnsureTensorSize(tex_tensor_, dim_sizes, stride_sizes, tensor_opts);

  const size_t width_pitch = long(desc.Width * desc.NumChannels * type_size);
  CudaSafeCall(cudaMemcpy2DFromArray(tex_tensor_.data_ptr(), width_pitch,
                                     tex_array, 0, 0, width_pitch,
                                     tex_tensor_.size(0), copy_kind));

  if (!non_blocking) CudaSafeCall(cudaDeviceSynchronize());
  CudaSafeCall(cudaGraphicsUnmapResources(1, &cuda_resource_));
  return tex_tensor_.squeeze();
}

torch::Tensor GLTexture::ToTensorGL() {
  vector<int64_t> dim_sizes;

  const auto dtype = cast_type<torch::ScalarType>(type_);
  if (target_ == GL_TEXTURE_2D || target_ == GL_TEXTURE_RECTANGLE) {
    const int depth = GLformatToDepth(format_);

    if (depth == 1) {
      dim_sizes = {dim_.get_height(), dim_.get_width()};
    } else {
      dim_sizes = {dim_.get_height(), dim_.get_width(), depth};
    }
  } else if (target_ == GL_TEXTURE_3D) {
    dim_sizes = {dim_.get_height(), dim_.get_width(), dim_.get_depth()};
  }

  torch::Tensor tex_tensor = torch::empty(dim_sizes, dtype);

  glBindTexture(target_, tex_);
  GLCheckError();

  glGetTexImage(target_, 0, format_, type_, tex_tensor.data_ptr());
  GLCheckError();

  glBindTexture(target_, 0);
  GLCheckError();

  return tex_tensor;
}

}  // namespace tenviz
