#pragma once
#include "vkg/math/glm_common.hpp"

namespace vkg {
struct Transform {
  glm::vec3 translation{0.f};
  glm::vec3 scale{1.f};
  glm::quat rotation{1, 0, 0, 0};

  Transform() = default;

  explicit Transform(
    const glm::vec3 &translation, const glm::vec3 &scale = glm::vec3{1.f},
    const glm::quat &rotation = {1, 0, 0, 0});

  explicit Transform(glm::mat4 m);

  auto toMatrix() const -> glm::mat4;
};

}
