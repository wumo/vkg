#pragma once
#include "aabb.hpp"
#include "vkg/base/vk_headers.hpp"
#include "vkg/render/ranges.hpp"
#include "vkg/render/allocation.hpp"
#include "vkg/render/model/vertex.hpp"
#include <span>

namespace vkg {
enum class PrimitiveTopology : uint32_t {
  Triangles = 1u,
  Lines = 2u,
  Procedural = 3u,
  Patches = 4u
};

enum class DynamicType : uint32_t { Static = 0u, Dynamic = 1u };

class Scene;
class Primitive {
public:
  struct Desc {
    UIntRange index, position, normal, uv;
    AABB aabb;
  };
  Primitive(
    Scene &scene, uint32_t id, UIntRange index, UIntRange position, UIntRange normal,
    UIntRange uv, PrimitiveTopology topology, DynamicType type);
  virtual auto id() const -> uint32_t;
  virtual auto index() const -> UIntRange;
  virtual auto position() const -> UIntRange;
  virtual auto normal() const -> UIntRange;
  virtual auto uv() const -> UIntRange;
  virtual auto aabb() const -> AABB;
  virtual auto setAABB(const AABB &aabb) -> void;
  virtual auto topology() const -> PrimitiveTopology;
  virtual auto type() const -> DynamicType;
  virtual auto descOffset() const -> uint32_t;

  auto update(std::span<Vertex::Position> positions, std::span<Vertex::Normal> normals)
    -> void;

protected:
  const uint32_t id_;
  const PrimitiveTopology topology_;
  const DynamicType type_;

  UIntRange index_, position_, normal_, uv_;
  AABB aabb_;

  UIntRange aabbRange_;

  Allocation<Desc> desc;
};
}
