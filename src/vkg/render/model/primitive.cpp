#include "primitive.hpp"
#include "vkg/render/scene.hpp"

namespace vkg {
Primitive::Primitive(
  Scene &scene, uint32_t id, std::vector<UIntRange> &&index,
  std::vector<UIntRange> &&position, std::vector<UIntRange> &&normal,
  std::vector<UIntRange> &&uv, const AABB &aabb, PrimitiveTopology topology,
  uint32_t count)
  : scene{scene}, id_{id}, count_{count}, topology_{topology} {
  frames.resize(count);
  for(auto i = 0u; i < count; i++) {
    frames[i] = {
      index[i], position[i], normal[i], uv[i], aabb, 0, scene.allocatePrimitiveDesc()};
    *frames[i].desc.ptr = {index[i], position[i], normal[i], uv[i], aabb, 0};
  }
}
auto Primitive::id() const -> uint32_t { return id_; }
auto Primitive::count() const -> uint32_t { return count_; }
auto Primitive::topology() const -> PrimitiveTopology { return topology_; }
auto Primitive::index(uint32_t idx) const -> UIntRange { return frames[idx].index_; }
auto Primitive::position(uint32_t idx) const -> UIntRange {
  return frames[idx].position_;
}
auto Primitive::normal(uint32_t idx) const -> UIntRange { return frames[idx].normal_; }
auto Primitive::uv(uint32_t idx) const -> UIntRange { return frames[idx].uv_; }
auto Primitive::aabb(uint32_t idx) const -> AABB { return frames[idx].aabb_; }

auto Primitive::descOffset() const -> uint32_t { return frames[0].desc.offset; }
void Primitive::setAABB(uint32_t idx, const AABB &aabb) {
  frames[idx].aabb_ = aabb;
  frames[idx].desc.ptr->aabb = aabb;
}
auto Primitive::update(
  uint32_t idx, std::span<Vertex::Position> positions, std::span<Vertex::Normal> normals,
  const AABB &aabb) -> void {
  //TODO check
  scene.Dev.positions->update(idx, frames[idx].position_, positions);
  scene.Dev.normals->update(idx, frames[idx].normal_, normals);
  setAABB(idx, aabb);
}
auto Primitive::update(uint32_t idx, PrimitiveBuilder &builder) -> void {
  errorIf(
    builder.primitives().size() != 1, "need only 1 primitive, but got ",
    builder.primitives().size());
  auto &p = builder.primitives()[0];
  errorIf(p.topology != topology_, "primitive topology or dynamicType shouldn't change");
  errorIf(
    p.position.size != frames[idx].position_.size,
    "updated positions.size != original.positions.size");
  errorIf(
    p.normal.size != frames[idx].normal_.size,
    "updated normals.size != original.normals.size");
  update(
    idx, builder.positions().subspan(p.position.start, p.position.size),
    builder.normals().subspan(p.normal.start, p.normal.size), p.aabb);
}
}
