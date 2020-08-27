#pragma once
#include <string>
#include <vector>
#include "transform.hpp"
#include "aabb.hpp"
#include "vkg/render/ranges.hpp"
#include "vkg/render/allocation.hpp"
#include <span>

namespace vkg {
class Scene;
class Node {
public:
  Node(Scene &scene, uint32_t id, const Transform &transform);
  auto id() const -> uint32_t;
  auto transform() const -> Transform;
  auto setTransform(const Transform &transform) -> void;
  auto name() const -> std::string;
  auto setName(const std::string &name) -> void;
  auto meshes() const -> std::span<const uint32_t>;
  auto addMeshes(std::vector<uint32_t> &&meshes) -> void;
  auto parent() const -> uint32_t;
  auto children() const -> std::span<const uint32_t>;
  auto addChildren(std::vector<uint32_t> &&children) -> void;
  auto aabb() -> AABB;
  auto freeze() -> void;
  auto transfOffset() const -> uint32_t;

private:
  Scene &scene;
  const uint32_t id_;

  Transform transform_;
  std::string name_;
  std::vector<uint32_t> meshes_;
  uint32_t parent_{nullIdx};
  std::vector<uint32_t> children_;
  AABB aabb_;
  bool frozen{false};

  Allocation<Transform> transf;
};
}