#pragma once

#include "gl_common.hpp"

#include <torch/torch.h>
#include "error.hpp"

namespace tenviz {

template <typename Type>
size_t GetTypeSize(Type type);

/**
 * Converts a torch scalar type to its size in bytes.
 */
template <>
inline size_t GetTypeSize(torch::ScalarType scalar_type) {
  switch (scalar_type) {
    case torch::kFloat64:
    case torch::kInt64:
      return 8;
    case torch::kFloat:
    case torch::kInt32:
      return 4;
    case torch::kHalf:
    case torch::kInt16:
      return 2;
    case torch::kUInt8:
    case torch::kInt8:
      return 1;
    default:
      throw Error("Tensor type is not supported");
  }
}

/**
 * Converts a OpenGL scalar type to its size in bytes.
 */
template <>
inline size_t GetTypeSize(GLenum scalar_type) {
  switch (scalar_type) {
    case GL_DOUBLE:
    case GL_INT64_ARB:
    case GL_UNSIGNED_INT64_ARB:
      return 8;

    case GL_FLOAT:
    case GL_INT:
    case GL_UNSIGNED_INT:
      return 4;

    case GL_HALF_FLOAT:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
      return 2;

    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
      return 1;

    default:
      throw Error("GLenum type is not compatible with OpenGL");
  }
}

template <typename To, typename From>
To cast_type(From type) {
  // Error!
  return type % 45.0f;
}

/**
 * Converts a torch scalar type to its OpenGL counterpart.
 */
template <>
inline GLenum cast_type(torch::ScalarType scalar_type) {
  switch (scalar_type) {
    case torch::kFloat64:
      return GL_DOUBLE;
    case torch::kInt64:
      return GL_INT64_ARB;

    case torch::kFloat:
      return GL_FLOAT;
    case torch::kInt32:
      return GL_INT;

    case torch::kHalf:
      return GL_HALF_FLOAT;
    case torch::kInt16:
      return GL_SHORT;

    case torch::kInt8:
      return GL_BYTE;
    case torch::kUInt8:
      return GL_UNSIGNED_BYTE;

    default:
      throw Error("Tensor type is not compatible with OpenGL");
  }
}

/**
 * Converts an OpenGL scalar type to its torch counterpart.
 */
template <>
inline torch::ScalarType cast_type(GLenum scalar_type) {
  switch (scalar_type) {
    case GL_DOUBLE:
      return torch::kFloat64;
    case GL_INT64_ARB:
      return torch::kInt64;
    case GL_FLOAT:
      return torch::kFloat;
    case GL_INT:
      return torch::kInt32;

    case GL_HALF_FLOAT:
      return torch::kHalf;
    case GL_SHORT:
      return torch::kInt16;

    case GL_BYTE:
      return torch::kInt8;
    case GL_UNSIGNED_BYTE:
      return torch::kUInt8;

    default:
      throw Error("Tensor type is not compatible with OpenGL");
  }
}

/**
 * Dynamic types supported by TensorViz's buffers.
 */
enum DType {
  kDouble = GL_DOUBLE,
  kInt64 = GL_INT64_ARB,
  kFloat = GL_FLOAT,
  kInt32 = GL_INT,
  kInt16 = GL_SHORT,
  kUint8 = GL_UNSIGNED_BYTE,
  kInt8 = GL_BYTE
};

/**
 * Converts a tensorviz scalar type to its OpenGL counterpart.
 */
template <>
inline GLenum cast_type(DType dtype) {
  return static_cast<GLenum>(dtype);
}

/**
 * Converts a tensorviz scalar type to its PyTorch counterpart.
 */
template <>
inline torch::ScalarType cast_type(DType dtype) {
  return cast_type<torch::ScalarType>(cast_type<GLenum>(dtype));
}

/**
 * Converts an OpenGL scalar type to its tensorviz counterpart.
 */
template <>
inline DType cast_type(GLenum gltype) {
  return static_cast<DType>(gltype);
}

/**
 * Converts an PyTorch scalar type to its tensorviz counterpart.
 */
template <>
inline DType cast_type(torch::ScalarType dtype) {
  return cast_type<DType>(cast_type<GLenum>(dtype));
}

}  // namespace tenviz
