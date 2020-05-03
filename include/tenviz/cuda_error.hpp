#pragma once

#include <sstream>

#include <cuda_runtime.h>

#include "error.hpp"

// Taken from:
// https://codeyarns.com/2011/03/02/how-to-do-error-checking-in-cuda/

#define CudaSafeCall(err) tenviz::_CudaSafeCall(err, __FILE__, __LINE__)
#define CudaCheck() tenviz::_CudaCheck(__FILE__, __LINE__)

namespace tenviz {
inline void _CudaSafeCall(cudaError err, const char *file, const int line) {
#ifndef NDEBUG
  if (err != cudaSuccess) {
    std::stringstream format;
    format << "Cuda call " << file << "(" << line
           << "): " << cudaGetErrorString(err);
    throw Error(format);
  }
#endif
}

inline void _CudaCheck(const char *file, const int line) {
#ifndef NDEBUG
  cudaError err = cudaGetLastError();
  if (cudaSuccess != err) {
    std::stringstream format;
    format << "Cuda call " << file << "(" << line
           << "): " << cudaGetErrorString(err);
    throw Error(format);
  }
#endif
}

}  // namespace tenviz
