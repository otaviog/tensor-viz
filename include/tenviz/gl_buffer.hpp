#pragma once

#include <inttypes.h>
#include <vector>

#include "gl_common.hpp"

#include <cuda_gl_interop.h>
#include <torch/torch.h>

#include "context_resource.hpp"
#include "cuda_memory.hpp"
#include "dtype.hpp"

namespace tenviz {

/**
 * Supported buffer targets.
 */
enum BufferTarget {
  kArray = GL_ARRAY_BUFFER,
  kElement = GL_ELEMENT_ARRAY_BUFFER
};

/**
 * Supported buffer usages.
 */
enum BufferUsage { kDynamic = GL_DYNAMIC_DRAW, kStatic = GL_STATIC_DRAW };

/**
 * OpenGL array buffer that can be mapped as PyTorch tensors.
 *
 * Instances should be registered into the context, see
 * IContextResource::RegisterResourceOnCurrent.
 */
class GLBuffer : public IContextResource {
 public:
  /**
   * Create an empty buffer.
   */
  static std::shared_ptr<GLBuffer> Create(BufferTarget target = kArray,
                                          BufferUsage usage = kDynamic) {
    auto buf = std::make_shared<GLBuffer>(target, usage);
    IContextResource::RegisterResourceOnCurrent(buf);
    return buf;
  }

  static void RegisterPybind(pybind11::module &m,
                             IContextResource::PythonClassDef &base_class);

  /**
   * @param target OpenGL's buffer target to use
   * @param usage OpenGL's usage
   */
  GLBuffer(GLenum target, GLenum usage);

  ~GLBuffer();

  GLBuffer(const GLBuffer &copy) = delete;

  GLBuffer &operator=(const GLBuffer &copy) = delete;

  void Release() override;

  /** 
   * Binds or unbids the buffer to the current OpenGL context.
   * 
   * @param do_binding true to bind, false to unbind.
   */
  void Bind(bool do_binding) const;

  /**
   * Allocate the buffer to have the given dimension and type.
   *
   * @param rows The buffer's number of rows.
   *
   * @param cols The buffer's number of cols. If 0, then the buffer is
   * interpreted as one dimension tensor. Columns size are restricted
   * to maximum of 4.
   *
   * @param type the target type.
   */
  void Allocate(int rows, int cols, DType type);

  /**
   * Map the buffer to a Tensor. The tensor must be unmapped by the
   * user.
   *
   * @return mapped tensor. 
   */
  std::shared_ptr<CudaMappedTensor> AsTensor();

  /**
   * Copies the tensor values to the buffer.
   */
  void FromTensor(const torch::Tensor &tensor);

  /**
   * Copies the buffer into a tensor.
   * 
   * @param keep_on_device Whatever the tensor should be kept on the
   * device. If the final Tensor target is the CPU, then GPU -> CPU
   * copying is faster.

   * @return A new tensor with the copied data from the buffer.
   */
  torch::Tensor ToTensor(bool keep_on_device = true);

  /**
   * Same as torch::index_put into the buffer.
   *
   * @param indices Selected indices.
   *
   * @param tensor The input tensor.
   */
  void IndexPut(const torch::Tensor &indices, const torch::Tensor &tensor);

  /**
   * Same as torch::index_select slicing operation.
   *
   * @param indices Selected indices.
   *
   * @param keep_on_device Whatever the tensor should be kept on the
   * device. If the final Tensor target is the CPU, then GPU -> CPU
   * copying is faster.
   *
   * @return A new tensor the copied data from the buffer.
   */
  torch::Tensor IndexSelect(torch::Tensor indices, bool keep_device = true);

  torch::Tensor IndexSelect_(torch::Tensor indices) {
    return IndexSelect(indices, true);
  }

  /**
   * @return The buffer dimensions.
   */
  std::vector<int64_t> get_size() const { return size_; }

  /**
   * @return The dimension's size.
   */
  long get_size(int idx) const { return size_[idx]; }

  /**
   * @return The number of dimensions.
   */
  int get_dim() const { return int(size_.size()); }

  /**
   * @return One of OpenGL's type (GL_FLOAT, GL_INT, ...) bounded to
   * this instance.
   */
  GLenum get_gl_type() const { return gltype_; }

  /**
   * @return OpenGL's creation ID.
   */
  GLuint get_buffer_id() const { return buffer_id_; }

  /**
   * @return Whatever if the tensor is empty.
   */
  bool is_empty() const { return size_.empty(); }
  
  bool normalize = false; /**< If true, the buffer will be normalized
                           * into 0.0-1.0 in shaders.*/
  bool integer_attrib = false; /**< If true, then types like Byte will
                                * not be converted to float in
                                * shaders..*/

 private:
  void AllocateImpl(size_t size);

  GLuint buffer_id_;
  cudaGraphicsResource_t cuda_resource_;
  GLenum target_, usage_, gltype_;
  std::vector<int64_t> size_;
};
}  // namespace tenviz
