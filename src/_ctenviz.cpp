#include <locale>
#include <memory>
#include <numeric>

#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <torch/python.h>
#include <torch/torch.h>

#include "gl_buffer.hpp"
#include "gl_common.hpp"
#include "gl_framebuffer.hpp"
#include "gl_shader_program.hpp"
#include "gl_texture.hpp"

#include "camera.hpp"
#include "draw_program.hpp"
#include "pose.hpp"
#include "projection.hpp"
#include "scene.hpp"
#include "se3.hpp"
#include "so3.hpp"
#include "style.hpp"
#include "viewer.hpp"

using namespace std;
namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(std::map<int, tenviz::FramebufferTarget>);

PYBIND11_MODULE(_ctenviz, m) {
  using namespace tenviz;

  tenviz::Error::RegisterPybind(m);
  tenviz::Context::RegisterPybind(m);
  tenviz::Viewer::RegisterPybind(m);

  auto ctx_resource = IContextResource::RegisterPybind(m);
  GLBuffer::RegisterPybind(m, ctx_resource);
  GLShaderProgram::RegisterPybind(m, ctx_resource);
  GLTexture::RegisterPybind(m, ctx_resource);
  GLFramebuffer::RegisterPybind(m, ctx_resource);

  auto node = ANode::RegisterPybind(m);
  Scene::RegisterPybind(m, node);
  DrawProgram::RegisterPybind(m, node);
  Style::RegisterPybind(m);

  BSphere::RegisterPybind(m);
  BBox3D::RegisterPybind(m);
  Bounds::RegisterPybind(m);

  SO3::RegisterPybind(m);
  SE3::RegisterPybind(m);
  Pose::RegisterPybind(m);

  /**
   * Camera
   */
  m.def("get_sphere_view", &camera::GetSphereView);
  m.def("get_min_bounding_distance", &camera::GetMinBoundingDistance);
}
