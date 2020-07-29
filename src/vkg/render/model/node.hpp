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
  virtual auto id() const -> uint32_t;
  virtual auto transform() const -> Transform;
  virtual auto setTransform(const Transform &transform) -> void;
  virtual auto name() const -> std::string;
  virtual auto setName(const std::string &name) -> void;
  virtual auto meshes() const -> std::span<const uint32_t>;
  virtual auto addMeshes(std::vector<uint32_t> &&meshes) -> void;
  virtual auto parent() const -> uint32_t;
  virtual auto children() const -> std::span<const uint32_t>;
  virtual auto addChildren(std::vector<uint32_t> &&children) -> void;
  virtual auto aabb() -> AABB;
  virtual auto freeze() -> void;
  virtual auto transfOffset() const -> uint32_t;

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