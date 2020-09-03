#pragma once
#include "aabb.hpp"
#include "vkg/base/vk_headers.hpp"
#include "vkg/render/ranges.hpp"
#include "vkg/render/allocation.hpp"
#include "vkg/render/model/vertex.hpp"
#include "vkg/base/resource/acc_structures.hpp"
#include "frame_updatable.hpp"
#include <span>

namespace vkg {
enum class PrimitiveTopology : uint32_t {
  Triangles = 1u,
  Lines = 2u,
  Procedural = 3u,
  Patches = 4u
};

class Scene;
class PrimitiveBuilder;
class Primitive: public FrameUpdatable {
public:
  struct Desc {
    UIntRange index, position, normal, uv;
    AABB aabb;
    uint64_t handle{0};
  };
  Primitive(
    Scene &scene, uint32_t id, std::vector<UIntRange> &&index,
    std::vector<UIntRange> &&position, std::vector<UIntRange> &&normal,
    std::vector<UIntRange> &&uv, const AABB &aabb, PrimitiveTopology topology,
    uint32_t count = 1);
  auto id() const -> uint32_t;
  auto count() const -> uint32_t;
  auto topology() const -> PrimitiveTopology;
  auto index(uint32_t idx) const -> UIntRange;
  auto position(uint32_t idx) const -> UIntRange;
  auto normal(uint32_t idx) const -> UIntRange;
  auto uv(uint32_t idx) const -> UIntRange;
  auto aabb(uint32_t idx) const -> AABB;
  void setAABB(uint32_t idx, const AABB &aabb);
  auto descOffset() const -> uint32_t;
  auto update(
    uint32_t idx, std::span<Vertex::Position> positions,
    std::span<Vertex::Normal> normals, const AABB &aabb) -> void;
  auto update(uint32_t idx, PrimitiveBuilder &builder) -> void;

protected:
  void updateFrame(uint32_t frameIdx, vk::CommandBuffer cb) override;

protected:
  Scene &scene;
  const uint32_t id_;
  const uint32_t count_;

  const PrimitiveTopology topology_;
  bool isRayTraced_{false};

  struct Frame {
    UIntRange index_, position_, normal_, uv_;
    AABB aabb_;
    ASDesc blas;

    Allocation<Desc> desc;
  };

  std::vector<Frame> frames;
};
}
