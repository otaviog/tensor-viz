#include "gl_buffer.hpp"

#include <limits>

#include <torch/csrc/utils/pybind.h>

#include "cuda_error.hpp"
#include "cuda_memory.hpp"
#include "gl_error.hpp"

using namespace std;

namespace tenviz {

class ScopedGLBufferMap {
 public:
  ScopedGLBufferMap(GLenum target, GLuint buffer, GLenum maptype)
      : target_(target), buffer_(buffer) {
    glBindBuffer(target, buffer);
    GLCheckError();
    data = glMapBuffer(target_, maptype);
    GLCheckError();
  }

  ~ScopedGLBufferMap() {
    glBindBuffer(target_, buffer_);
    GLCheckError();
    glUnmapBuffer(target_);
    GLCheckError();
    glBindBuffer(target_, 0);
    GLCheckError();
  }

  void *data;

 private:
  GLenum target_;
  GLuint buffer_;
};

namespace {
const GLuint GL_SENTINEL = numeric_limits<GLuint>::max();

}  // namespace

void GLBuffer::RegisterPybind(pybind11::module &m,
                              IContextResource::PythonClassDef &base_class) {
  py::class_<GLBuffer, shared_ptr<GLBuffer>>(m, "Buffer", base_class)
      .def(py::init<BufferTarget, BufferUsage>())
      .def("from_tensor", &GLBuffer::FromTensor)
      .def("to_tensor", &GLBuffer::ToTensor, py::arg("keep_on_device") = true)
      .def("allocate", &GLBuffer::Allocate)
      .def("__setitem__", &GLBuffer::IndexPut)
      .def("__getitem__", &GLBuffer::IndexSelect_)
      .def("as_tensor", &GLBuffer::AsTensor)
      .def_readwrite("normalize", &GLBuffer::normalize)
      .def_readwrite("integer_attrib", &GLBuffer::integer_attrib);

  py::class_<CudaMappedTensor, shared_ptr<CudaMappedTensor>>(m,
                                                             "CudaMappedTensor")
      .def("unmap", &CudaMappedTensor::Unmap)
      .def_readwrite("tensor", &CudaMappedTensor::tensor);

  py::enum_<BufferTarget>(m, "BufferTarget")
      .value("Array", kArray)
      .value("Element", kElement)
      .export_values();
  py::enum_<BufferUsage>(m, "BufferUsage")
      .value("Dynamic", kDynamic)
      .value("Static", kStatic)
      .export_values();
  py::enum_<DType>(m, "DType")
      .value("Double", DType::kDouble)
      .value("Int64", DType::kInt64)
      .value("Float", DType::kFloat)
      .value("Int32", DType::kInt32)
      .value("Int16", DType::kInt16)
      .value("Uint8", DType::kUint8)
      .value("Int8", DType::kInt8)
      .export_values();
}

GLBuffer::GLBuffer(GLenum target, GLenum usage) {
  glGenBuffers(1, &buffer_id_);
  GLCheckError();

  target_ = target;
  gltype_ = GL_NONE;
  usage_ = usage;
  cuda_resource_ = nullptr;
  normalize = false;
  integer_attrib = false;
}

GLBuffer::~GLBuffer() { Release(); }

void GLBuffer::Release() {
  if (buffer_id_ == GL_SENTINEL) {
    return;
  }

  if (cuda_resource_) {
    CudaSafeCall(cudaGraphicsUnregisterResource(cuda_resource_));
    cuda_resource_ = nullptr;
  }

  glDeleteBuffers(1, &buffer_id_);
  GLCheckError();

  buffer_id_ = GL_SENTINEL;
}

void GLBuffer::Bind(bool do_binding) const {
  if (do_binding) {
    glBindBuffer(target_, buffer_id_);
    GLCheckError();
  } else {
    glBindBuffer(target_, 0);
    GLCheckError();
  }
}

void GLBuffer::Allocate(int rows, int cols, DType btype) {
  GLenum gltype = cast_type<GLenum>(btype);
  torch::ScalarType type = cast_type<torch::ScalarType>(btype);

  const size_t size = GetTypeSize(type) * rows * ((cols > 0) ? cols : 1);
  if (size > 0) {
    AllocateImpl(size);
    gltype_ = gltype;
    if (cols > 0)
      size_ = {rows, cols};
    else
      size_ = {rows};
  }
}

void GLBuffer::AllocateImpl(size_t size) {
  Bind(true);
  glBufferData(target_, size, nullptr, usage_);
  GLCheckError();
  Bind(false);
  if (cuda_resource_ == nullptr) {
    CudaSafeCall(cudaGraphicsGLRegisterBuffer(&cuda_resource_, buffer_id_,
                                              cudaGraphicsMapFlagsNone));
  }
}

std::shared_ptr<CudaMappedTensor> GLBuffer::AsTensor() {
  void *data;
  size_t size;

  CudaSafeCall(cudaGraphicsMapResources(1, &cuda_resource_));
  CudaSafeCall(
      cudaGraphicsResourceGetMappedPointer(&data, &size, cuda_resource_));

  auto map = make_shared<CudaMappedTensor>(cuda_resource_);
  auto opts = torch::TensorOptions(torch::kCUDA, 0)
                  .dtype(cast_type<torch::ScalarType>(gltype_));
  map->tensor = torch::from_blob(data, size_, opts);

  return map;
}

void GLBuffer::FromTensor(const torch::Tensor &tensor) {
  gltype_ = cast_type<GLenum>(tensor.scalar_type());

  auto sizes = tensor.sizes();
  size_.clear();
  copy(sizes.begin(), sizes.end(), back_inserter(size_));

  const size_t size = GetTypeSize(gltype_) * tensor.numel();

  if (size == 0) {
    return;
  }

  AllocateImpl(size);

  ScopedCudaMapper map(cuda_resource_);
  if (tensor.device().is_cpu()) {
    cudaMemcpy(map.get(), tensor.data_ptr(), size, cudaMemcpyHostToDevice);
  } else {
    cudaMemcpy(map.get(), tensor.data_ptr(), size, cudaMemcpyDeviceToDevice);
  }
}

void CUDAIndexPut(const torch::Tensor &indices, const torch::Tensor &tensor,
                  void *buffer_data);

namespace {
template <typename ScalarType>
void CPUIndexPut(const torch::Tensor &_indices, const torch::Tensor &_tensor,
                 void *_gl_data) {
  ScalarType *gl_data = reinterpret_cast<ScalarType *>(_gl_data);
  const auto tensor = _tensor.cpu();
  const auto tensor_a = tensor.accessor<ScalarType, 2>();

  auto indices = _indices.cpu();
  auto idx_a = indices.accessor<int64_t, 1>();

  for (int i = 0; i < indices.size(0); ++i) {
    const int idx = idx_a[i];
    const size_t offset = tensor.size(1) * idx;

    for (int j = 0; j < tensor.size(1); ++j) {
      gl_data[offset + j] = tensor_a[i][j];
    }
  }
}
}  // namespace

void GLBuffer::IndexPut(const torch::Tensor &dst_indices,
                        const torch::Tensor &_tensor) {
  if (dst_indices.size(0) != _tensor.size(0)) {
    throw Error("Put index and tensor must match sizes");
  }

  if (dst_indices.device() != _tensor.device()) {
    throw Error("Indice tensor and value tensor device must match");
  }

  if (dst_indices.size(0) == 0) {
    return;
  }

  int cols = 1;
  if (size_.size() == 2) {
    cols = size_[1];
  }

  const torch::Tensor tensor = _tensor.view({-1, cols});
  if (_tensor.device().is_cuda()) {
    ScopedCudaMapper map(cuda_resource_);

    CUDAIndexPut(dst_indices, tensor, map.get());
  } else {
    ScopedGLBufferMap gl_map(target_, buffer_id_, GL_WRITE_ONLY);

    auto type_id = tensor.options().dtype();
    AT_DISPATCH_ALL_TYPES(tensor.scalar_type(), "CPUIndexPut", ([&] {
                            CPUIndexPut<scalar_t>(dst_indices.cpu(), tensor,
                                                  gl_map.data);
                          }));
  }
}

torch::Tensor GLBuffer::ToTensor(bool keep_on_device) {
  int total_size = 1;
  for (int dimsize : size_) {
    total_size *= dimsize;
  }

  total_size = GetTypeSize(gltype_) * total_size;

  const torch::ScalarType dtype = cast_type<torch::ScalarType>(gltype_);
  auto opts = torch::TensorOptions().dtype(dtype);

  cudaMemcpyKind cpyKind = cudaMemcpyDeviceToHost;
  if (keep_on_device) {
    opts = opts.device(torch::kCUDA, 0);
    cpyKind = cudaMemcpyDeviceToDevice;
  }
  torch::Tensor tensor = torch::empty(size_, opts);
  ScopedCudaMapper map(cuda_resource_);
  cudaMemcpy(tensor.data_ptr(), map.get(), total_size, cpyKind);
  return tensor;
}

namespace {
template <typename ScalarType>
void CPUIndexSelect(ScalarType *gl_data, const torch::Tensor &_indices,
                    torch::Tensor &tensor) {
  auto tensor_a = tensor.accessor<ScalarType, 2>();

  const auto indices = _indices.cpu();
  auto idx_a = indices.accessor<int64_t, 1>();

  for (int i = 0; i < indices.size(0); ++i) {
    const int idx = idx_a[i];
    const size_t offset = tensor.size(1) * idx;

    for (int j = 0; j < tensor.size(1); ++j) {
      tensor_a[i][j] = gl_data[offset + j];
    }
  }
}
}  // namespace

void CUDAIndexSelect(const torch::Tensor &indices, const void *buffer_data,
                     torch::Tensor &tensor);

torch::Tensor GLBuffer::IndexSelect(torch::Tensor indices,
                                    bool keep_on_device) {
  torch::Tensor result;
  torch::ScalarType dtype = cast_type<torch::ScalarType>(gltype_);

  int cols = 1;
  if (get_dim() == 2) cols = get_size(1);

  const int64_t result_dims[] = {indices.size(0), cols};

  if (indices.size(0) == 0) {
    return torch::empty(result_dims, torch::TensorOptions(dtype));
  }

  if (keep_on_device) {
    if (!indices.is_cuda()) {
      throw Error(
          "Indices tensor must be on GPU if `keep_on_device` is `true`.");
    }

    result = torch::empty(
        result_dims,
        torch::TensorOptions().device(torch::kCUDA, 0).dtype(dtype));

    ScopedCudaMapper map(cuda_resource_);
    CUDAIndexSelect(indices, map.get(), result);
  } else {
    if (indices.is_cuda()) {
      throw Error(
          "Indices tensor must be on CPU if `keep_on_device` is `true`.");
    }
    result = torch::empty(result_dims, dtype);

    ScopedGLBufferMap glmap(target_, buffer_id_, GL_READ_ONLY);
    AT_DISPATCH_ALL_TYPES(result.scalar_type(), "CPUIndexSelect", ([&] {
                            CPUIndexSelect<scalar_t>(
                                reinterpret_cast<scalar_t *>(glmap.data),
                                indices, result);
                          }));
  }

  if (get_dim() == 1) result = result.squeeze();
  return result;
}

}  // namespace tenviz
