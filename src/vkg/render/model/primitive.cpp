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
    auto &frame = frames[i];
    frame = {
      index[i], position[i], normal[i], uv[i], aabb, {}, scene.allocatePrimitiveDesc()};
    *frame.desc.ptr = {index[i], position[i], normal[i],
                       uv[i],    aabb,        frames[i].blas.handle};
  }
  if(scene.featureConfig.rayTrace) {
    switch(topology) {
      case PrimitiveTopology::Triangles:
        isRayTraced_ = true;
        scene.scheduleFrameUpdate(Update::Type::Primitive, id_, count_, ticket);
        break;
      default: break;
    }
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
  scene.scheduleFrameUpdate(Update::Type::Primitive, id_, count_, ticket);
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
void Primitive::updateFrame(uint32_t frameIdx, vk::CommandBuffer cb) {
  if(!isRayTraced_) return;
  auto &frame = frames[frameIdx];

  auto posBufInfo = scene.Dev.positions->bufferInfo();
  auto indexBufInfo = scene.Dev.indices->bufferInfo();

  frame.blas.type = vk::AccelerationStructureTypeNV::eBottomLevel;
  frame.blas.flags = vk::BuildAccelerationStructureFlagBitsNV::ePreferFastTrace |
                     vk::BuildAccelerationStructureFlagBitsNV::eAllowCompaction |
                     vk::BuildAccelerationStructureFlagBitsNV::eAllowUpdate;
  vk::GeometryNV geometry{
    vk::GeometryTypeNV::eTriangles,
    vk::GeometryDataNV{vk::GeometryTrianglesNV{
      posBufInfo.buffer,
      posBufInfo.offset + frame.position_.start * sizeof(Vertex::Position),
      frame.position_.size, sizeof(Vertex::Position), vk::Format::eR32G32B32Sfloat,
      indexBufInfo.buffer, indexBufInfo.offset + frame.index_.start * sizeof(uint32_t),
      frame.index_.size, vk::IndexType::eUint32}},
    vk::GeometryFlagBitsNV::eOpaque};
  if(!frame.blas.as) {
    allocAS(scene.device, frame.blas, 0, 1, &geometry);
    frame.desc.ptr->handle = frame.blas.handle;
    auto scratchBuffer = allocBuildScratchBuffer(scene.device, *frame.blas.as);
    auto scratchBufferInfo = scratchBuffer->bufferInfo();
    vk::AccelerationStructureInfoNV info{
      frame.blas.type, frame.blas.flags, 0, 1, &geometry};

    cb.buildAccelerationStructureNV(
      info, nullptr, 0, false, *frame.blas.as, nullptr, scratchBufferInfo.buffer,
      scratchBufferInfo.offset);
    cb.pipelineBarrier(
      vk::PipelineStageFlagBits::eAccelerationStructureBuildNV,
      vk::PipelineStageFlagBits::eRayTracingShaderNV, {},
      vk::MemoryBarrier{
        vk::AccessFlagBits::eAccelerationStructureWriteNV,
        vk::AccessFlagBits::eAccelerationStructureReadNV},
      nullptr, nullptr);
  } else {
    if(
      !frame.blas.scratchBuffer || updateScratchBufferSize(scene.device, *frame.blas.as) >
                                     frame.blas.scratchBuffer->bufferInfo().size)
      frame.blas.scratchBuffer =
        allocUpdateScratchBuffer(scene.device, *frame.blas.as, "blasUpdateScratchBuffer");
    auto scratchBufferInfo = frame.blas.scratchBuffer->bufferInfo();
    vk::AccelerationStructureInfoNV info{
      frame.blas.type, frame.blas.flags, 0, 1, &geometry};

    cb.buildAccelerationStructureNV(
      info, nullptr, 0, true, *frame.blas.as, *frame.blas.as, scratchBufferInfo.buffer,
      scratchBufferInfo.offset);
    cb.pipelineBarrier(
      vk::PipelineStageFlagBits::eAccelerationStructureBuildNV,
      vk::PipelineStageFlagBits::eRayTracingShaderNV, {},
      vk::MemoryBarrier{
        vk::AccessFlagBits::eAccelerationStructureWriteNV,
        vk::AccessFlagBits::eAccelerationStructureReadNV},
      nullptr, nullptr);
  }
}
}
