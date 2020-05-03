#pragma once

#include <cuda_gl_interop.h>
#include <torch/torch.h>

#include "cuda_error.hpp"

namespace tenviz {
/**
 * RAII for mapping/unmapping cudaGraphicsResource_t.
 */
class ScopedCudaMapper {
 public:
  ScopedCudaMapper(cudaGraphicsResource_t resource) : cuda_resource(resource) {
    CudaSafeCall(cudaGraphicsMapResources(1, &cuda_resource));
    CudaSafeCall(
        cudaGraphicsResourceGetMappedPointer(&data, &size, cuda_resource));
  };

  ~ScopedCudaMapper() {
    CudaSafeCall(cudaGraphicsUnmapResources(1, &cuda_resource, 0));
  }

  /**
   * @return Data pointer to the device memory.
   */
  void *get() { return data; }

 private:
  cudaGraphicsResource_t cuda_resource;
  void *data;
  size_t size;
};  // namespace

/**
 * Holds a PyTorch tensor created from a Graphics Resource data.
 */
class CudaMappedTensor {
 public:
  CudaMappedTensor(cudaGraphicsResource_t cuda_resource)
      : cuda_resource_(cuda_resource) {}

  torch::Tensor tensor; /**<Mapped tensor. */

  /**
   * Call this after using.
   */ 
  void Unmap() {
    CudaSafeCall(cudaGraphicsUnmapResources(1, &cuda_resource_, 0));
  }

 private:
  cudaGraphicsResource_t cuda_resource_;
};

}  // namespace tenviz
