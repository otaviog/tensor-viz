#pragma once

#include <memory>

#include <torch/csrc/utils/pybind.h>

namespace tenviz {
/**
 * Base class for graphics library resources.
 */
class IContextResource {
 public:
  typedef pybind11::class_<IContextResource, std::shared_ptr<IContextResource>>
      PythonClassDef;

  static PythonClassDef RegisterPybind(pybind11::module &m);

  /**
   * Register a resource (classes that derive IContextResource) into
   * the context that is currently associated to the thread.
   */
  static void RegisterResourceOnCurrent(
      std::shared_ptr<IContextResource> resource);

  /**
   * Deletes the resource. The context is make current by the caller.
   */
  virtual void Release() = 0;

 private:
};
}  // namespace tenviz
