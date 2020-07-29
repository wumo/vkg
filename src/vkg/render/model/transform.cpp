#include "transform.hpp"

namespace vkg {
Transform::Transform(
  const glm::vec3 &translation, const glm::vec3 &scale, const glm::quat &rotation)
  : translation(translation), scale(scale), rotation(rotation) {}

Transform::Transform(glm::mat4 m) {
  translation = glm::vec3(m[3]);
  scale = {length(m[0]), length(m[1]), length(m[2])};
  m = glm::scale(m, glm::vec3(1 / scale.x, 1 / scale.y, 1 / scale.z));
  m[3] = glm::vec4(0, 0, 0, 1);
  rotation = quat_cast(m);
}

auto Transform::toMatrix() const -> glm::mat4 {
  glm::mat4 m{1.0};
  m = glm::scale(m, scale);
  m = glm::mat4_cast(rotation) * m;
  m[3] = glm::vec4(translation, 1);
  return m;
}
}