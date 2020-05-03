#include "context_resource.hpp"

#include "context.hpp"

using namespace std;

namespace tenviz {
void IContextResource::RegisterResourceOnCurrent(
    std::shared_ptr<IContextResource> resource) {
  Context::RegisterResourceOnCurrent(resource);
}

IContextResource::PythonClassDef IContextResource::RegisterPybind(
    pybind11::module &m) {
  pybind11::class_<IContextResource, shared_ptr<IContextResource>> class_def(
      m, "ContextResource");
  class_def.def_static("register_resource",
                       &IContextResource::RegisterResourceOnCurrent);

  m.def("register_resource", &IContextResource::RegisterResourceOnCurrent);
  return class_def;
}

}  // namespace tenviz
