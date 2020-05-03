#include <torch/torch.h>

#include "cuda_error.hpp"
#include "error.hpp"

namespace tenviz {
template <typename scalar_t>
__global__ void KernIndexPut(
    const torch::PackedTensorAccessor<int64_t, 1, torch::RestrictPtrTraits,
                                      size_t>
        indices,
    const torch::PackedTensorAccessor<scalar_t, 2, torch::RestrictPtrTraits,
                                      size_t>
        tdata,
    scalar_t *bdata) {
  const int indices_idx = blockIdx.x;
  if (indices_idx < indices.size(0)) {
    const int idx = indices[indices_idx];
    const int col_idx = threadIdx.x;
    if (col_idx < tdata.size(1)) {
      const size_t offset = tdata.size(1) * idx + col_idx;
      bdata[offset] = tdata[indices_idx][col_idx];
    }
  }
}

template <typename scalar_t>
__global__ void KernIndexSelect(
    const torch::PackedTensorAccessor<int64_t, 1, torch::RestrictPtrTraits,
                                      size_t>
        indices,
    const scalar_t *gl_data,
    torch::PackedTensorAccessor<scalar_t, 2, torch::RestrictPtrTraits, size_t>
        tensor) {
  const int tensor_idx = blockIdx.x;
  if (tensor_idx < indices.size(0)) {
    const int col_idx = threadIdx.x;
    if (col_idx < tensor.size(1)) {
      const int buffer_idx = indices[tensor_idx];
      const int offset = tensor.size(1) * buffer_idx + col_idx;
      tensor[tensor_idx][col_idx] = gl_data[offset];
    }
  }
}

void CUDAIndexPut(const torch::Tensor &indices, const torch::Tensor &tensor,
                  void *buffer_data) {
  int blk_sz = indices.size(0);
  int thd_sz = tensor.size(1);
  const auto indices_a =
      indices.packed_accessor<int64_t, 1, torch::RestrictPtrTraits, size_t>();

  AT_DISPATCH_ALL_TYPES(
      tensor.scalar_type(), "CUDAIndexPut", ([&] {
        KernIndexPut<scalar_t><<<blk_sz, thd_sz>>>(
            indices_a,
            tensor.packed_accessor<scalar_t, 2, torch::RestrictPtrTraits,
                                   size_t>(),
            reinterpret_cast<scalar_t *>(buffer_data));
      }));
  CudaCheck();
  CudaSafeCall(cudaDeviceSynchronize());
}

void CUDAIndexSelect(const torch::Tensor &indices, const void *gl_data,
                     torch::Tensor &tensor) {
  const int blk_sz = indices.size(0);
  const int thd_sz = tensor.size(1);

  const auto indices_a =
      indices.packed_accessor<int64_t, 1, torch::RestrictPtrTraits, size_t>();
  AT_DISPATCH_ALL_TYPES(
      tensor.scalar_type(), "CUDAIndexSelect", ([&] {
        KernIndexSelect<scalar_t><<<blk_sz, thd_sz>>>(
            indices_a, reinterpret_cast<const scalar_t *>(gl_data),
            tensor.packed_accessor<scalar_t, 2, torch::RestrictPtrTraits,
                                   size_t>());
      }));
  CudaCheck();
  CudaSafeCall(cudaDeviceSynchronize());
}
}  // namespace tenviz