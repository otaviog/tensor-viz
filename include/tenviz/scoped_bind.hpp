#pragma once

#include <memory>

namespace tenviz {
template <typename Type>

/**
 * RAII for instances that have a Bind(bool do_binding) method.
 */
class ScopedBind {
 public:
  ScopedBind(std::shared_ptr<Type> res) : res_(res) { res_->Bind(true); }

  ~ScopedBind() { res_->Bind(false); }

 private:
  std::shared_ptr<Type> res_;
};
}  // namespace tenviz
