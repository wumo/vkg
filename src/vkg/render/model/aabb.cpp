#include "aabb.hpp"

namespace vkg {
auto AABB::transform(glm::mat4 m) const -> AABB {
  auto _min = glm::vec3(m[3]);
  auto _max = _min;

  auto p = glm::vec3(m[0]);
  auto v0 = p * min.x;
  auto v1 = p * max.x;
  _min += glm::min(v0, v1);
  _max += glm::max(v0, v1);

  p = glm::vec3(m[1]);
  v0 = p * min.y;
  v1 = p * max.y;
  _min += glm::min(v0, v1);
  _max += glm::max(v0, v1);

  p = glm::vec3(m[2]);
  v0 = p * min.z;
  v1 = p * max.z;
  _min += glm::min(v0, v1);
  _max += glm::max(v0, v1);

  return {_min, _max};
}

auto AABB::transform(Transform trans) const -> AABB {
  return transform(trans.toMatrix());
}

auto AABB::merge(glm::vec3 p) -> void {
  min = glm::min(min, p);
  max = glm::max(max, p);
}

auto AABB::merge(AABB other) -> void {
  min = glm::min(min, other.min);
  max = glm::max(max, other.max);
}

auto AABB::center() const -> glm::vec3 { return (min + max) / 2.f; }

auto AABB::halfRange() const -> glm::vec3 { return glm::abs(max - min) / 2.f; }
auto AABB::range() const -> glm::vec3 { return glm::abs(max - min); }

auto operator<<(std::ostream &os, const AABB &aabb) -> std::ostream & {
  os << "min: " << glm::to_string(aabb.min) << " max: " << glm::to_string(aabb.max);
  return os;
}
}