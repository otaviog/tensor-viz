
#include <memory>
#include "gl_common.hpp"

#include "bsphere.hpp"
#include "gl_buffer.hpp"
#include "math.hpp"

using namespace std;

namespace tenviz {
namespace {
class UnitSphere {
 public:
  const static int TICKS = 100;

  UnitSphere() : m_points(GL_ARRAY_BUFFER, GL_STATIC_DRAW) {
    torch::Tensor points = torch::zeros({TICKS * 3, 3}, torch::kFloat);
    auto points_a = points.accessor<float, 2>();
    int point_count = 0;

    // first and second rings
    for (int i = 0; i < TICKS; i++) {
      float theta = (i / float(TICKS - 1)) * math::kFloatPI * 2.0f;
      float phi = 0.0;

      points_a[i][0] = cos(phi) * cos(theta);
      points_a[i][1] = cos(phi) * sin(theta);
      points_a[i][2] = 0.0f;

      phi = math::kFloatPI * 0.5f;
      points_a[TICKS + i][0] = 0.0f;
      points_a[TICKS + i][1] = sin(theta);
      points_a[TICKS + i][2] = cos(theta);

      int offset = TICKS * 2;
      points_a[offset + i][0] = sin(theta);
      points_a[offset + i][1] = 0.0f;
      points_a[offset + i][2] = cos(theta);
    }

    m_points.FromTensor(points);
  }

  void Draw() {
    glEnableClientState(GL_VERTEX_ARRAY);
    m_points.Bind(true);
    glVertexPointer(3, GL_FLOAT, 0, nullptr);

    glLineWidth(2.0);
    glColor3f(0.0f, 1.0f, 0.0f);
    glDrawArrays(GL_LINE_STRIP, 0, TICKS);

    glColor3f(0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINE_STRIP, TICKS, TICKS);

    glColor3f(1.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINE_STRIP, TICKS * 2, TICKS);

    m_points.Bind(false);
    glDisableClientState(GL_VERTEX_ARRAY);
  }

 private:
  GLBuffer m_points;
};

unique_ptr<UnitSphere> g_unit_sphere = nullptr;
}  // namespace

void DrawSphere(const BSphere &sphere, const Eigen::Matrix4f &projection,
                const Eigen::Matrix4f &view) {
  if (!g_unit_sphere) {
    g_unit_sphere.reset(new UnitSphere);
  }

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(projection.data());

  glMatrixMode(GL_MODELVIEW);
  Eigen::Affine3f transf = Eigen::Affine3f::Identity();
  transf.translate(sphere.get_center());
  transf.scale(sphere.get_radius());

  Eigen::Matrix4f view_mtx = view * transf.matrix();
  glLoadMatrixf(view_mtx.data());

  g_unit_sphere->Draw();
}
}  // namespace tenviz
