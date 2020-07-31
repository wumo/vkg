#pragma once
#include "glm_common.hpp"
#include <array>

namespace vkg {
struct Frustum {
  std::array<glm::vec4, 6> planes;

  explicit Frustum(glm::mat4 m);
  auto update(glm::mat4 m) -> void;
  auto contains(glm::vec3 p) -> bool;
};
}
