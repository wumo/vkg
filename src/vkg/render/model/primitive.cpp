#include "primitive.hpp"
#include "vkg/render/scene.hpp"

namespace vkg {
Primitive::Primitive(
  Scene &scene, uint32_t id, UIntRange index, UIntRange position, UIntRange normal,
  UIntRange uv, PrimitiveTopology topology, DynamicType type)
  : scene{scene},
    id_{id},
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
  uint32_t queueIdx, std::span<Vertex::Position> positions,
  std::span<Vertex::Normal> normals) -> void {
  errorIf(
    type_ != DynamicType::Dynamic,
    "only Dynamic type primitive can be updated using this function");
  scene.Dev.positions->update(queueIdx, position_, positions);
  scene.Dev.normals->update(queueIdx, normal_, normals);
}
auto Primitive::update(uint32_t queueIdx, PrimitiveBuilder &builder) -> void {
  errorIf(
    builder.primitives().size() != 1, "need only 1 primitive, but got ",
    builder.primitives().size());
  auto &p = builder.primitives()[0];
  errorIf(
    p.topology != topology_ || p.type != type_,
    "primitive topology or dynamicType shouldn't change");
  errorIf(
    p.position.size != position_.size,
    "updated positions.size != original.positions.size");
  errorIf(p.normal.size != normal_.size, "updated normals.size != original.normals.size");
  update(
    queueIdx, builder.positions().subspan(p.position.start, p.position.size),
    builder.normals().subspan(p.normal.start, p.normal.size));
  setAABB(p.aabb);
}
}
