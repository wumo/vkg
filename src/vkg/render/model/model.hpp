#pragma once
#include <cstdint>
#include "aabb.hpp"
#include "animation.hpp"
#include "vkg/render/ranges.hpp"
#include "node.hpp"
#include <span>

namespace vkg {
class Scene;
class Model {
public:
  explicit Model(
    Scene &scene, uint32_t id, const std::vector<uint32_t> &nodes,
    std::vector<Animation> &&animations = {});
  virtual auto id() const -> uint32_t;
  virtual auto nodes() const -> std::span<const uint32_t>;
  virtual auto aabb() -> AABB;
  virtual auto animations() -> std::span<Animation>;

protected:
  const uint32_t id_;
  std::vector<uint32_t> nodes_;
  std::vector<Animation> animations_;
  AABB aabb_;
};
}
