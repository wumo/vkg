#include "primitive.hpp"
#include "vkg/render/scene.hpp"

namespace vkg {
Primitive::Primitive(
  Scene &scene, uint32_t id, UIntRange index, UIntRange position, UIntRange normal,
  UIntRange uv, PrimitiveTopology topology, DynamicType type)
  : id_{id},
    index_{index},
    position_{position},
    normal_{normal},
    uv_{uv},
    topology_{topology},
    type_{type},
    desc{scene.allocatePrimitiveDesc()} {
  *desc.ptr = {index, position, normal, uv, aabb_};
}
auto Primitive::id() const -> uint32_t { return id_; }
auto Primitive::index() const -> UIntRange { return index_; }
auto Primitive::position() const -> UIntRange { return position_; }
auto Primitive::normal() const -> UIntRange { return normal_; }
auto Primitive::uv() const -> UIntRange { return uv_; }
auto Primitive::aabb() const -> AABB { return aabb_; }
auto Primitive::topology() const -> PrimitiveTopology { return topology_; }
auto Primitive::type() const -> DynamicType { return type_; }
auto Primitive::descOffset() const -> uint32_t { return desc.offset; }

auto Primitive::setAABB(const AABB &aabb) -> void {
  aabb_ = aabb;
  desc.ptr->aabb = aabb;
}
auto Primitive::update(
  std::span<Vertex::Position> positions, std::span<Vertex::Normal> normals) -> void {}
}
