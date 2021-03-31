#pragma once

#include <exception>
#include <sstream>
#include <string>

#include <torch/csrc/utils/pybind.h>

namespace tenviz {

/* Handles error cases in TensorViz. Besides being derived from
 * std::exception, it can be used by passing as reference
 * parameter.
 */
class Error : public std::exception {
 public:
  static void RegisterPybind(pybind11::module &m);

  Error() { error_ = false; }

  /* @message error message.
   */
  Error(const std::string &message) : message_(message) { error_ = true; }

  /* std::stringstream overload.
   * @stream string stream
   */
  Error(const std::stringstream &stream) : message_(stream.str()) {
    error_ = true;
  }

  const char *what() const noexcept { return message_.c_str(); }

  operator bool() const { return error_; }

 private:
  bool error_;
  std::string message_;
};
}  // namespace tenviz
