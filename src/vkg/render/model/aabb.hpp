#pragma once
#include "vkg/math/glm_common.hpp"
#include "transform.hpp"
#include <ostream>

namespace vkg {
struct AABB {
  glm::vec3 min{std::numeric_limits<float>::infinity()};
  glm::vec3 max{-std::numeric_limits<float>::infinity()};

  auto transform(glm::mat4 m) const -> AABB;
  auto transform(Transform transform) const -> AABB;

  template<typename... Args>
  auto merge(glm::vec3 p, Args &&... points) -> void {
    merge(p);
    merge(points...);
  }

  auto merge(glm::vec3 p) -> void;

  auto merge(AABB other) -> void;

  auto center() const -> glm::vec3;

  auto halfRange() const -> glm::vec3;

  auto range() const -> glm::vec3;

  friend auto operator<<(std::ostream &os, const AABB &aabb) -> std::ostream &;
};
}
