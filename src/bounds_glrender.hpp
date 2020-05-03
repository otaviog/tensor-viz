#pragma once

#include "eigen_common.hpp"

namespace tenviz {

class BSphere;

void DrawSphere(const BSphere &sphere, const Eigen::Matrix4f &projection,
                const Eigen::Matrix4f &view);

}  // namespace tenviz
