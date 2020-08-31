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
    if(scene.featureConfig.rayTrace) {
      switch(topology) {
        case PrimitiveTopology::Triangles: {
          isRayTraced_ = true;
          auto posBufInfo = scene.Dev.positions->bufferInfo();
          auto indexBufInfo = scene.Dev.indices->bufferInfo();

          frames[i].blas.type = vk::AccelerationStructureTypeNV::eBottomLevel;
          frames[i].blas.flags =
            vk::BuildAccelerationStructureFlagBitsNV::ePreferFastTrace |
            vk::BuildAccelerationStructureFlagBitsNV::eAllowCompaction;
          frames[i].blas.geometry = {
            vk::GeometryTypeNV::eTriangles,
            vk::GeometryDataNV{vk::GeometryTrianglesNV{
              posBufInfo.buffer,
              posBufInfo.offset + position[i].start * sizeof(Vertex::Position),
              position[i].size, sizeof(Vertex::Position), vk::Format::eR32G32B32Sfloat,
              indexBufInfo.buffer,
              indexBufInfo.offset + index[i].start * sizeof(uint32_t), index[i].size,
              vk::IndexType::eUint32}},
            vk::GeometryFlagBitsNV::eOpaque};
          allocAS(scene.device, frames[i].blas, 0, 1, &frames[i].blas.geometry);
          buildAS(i);
        } break;
        default: break;
      }
    }
    *frame.desc.ptr = {index[i], position[i], normal[i],
                       uv[i],    aabb,        frames[i].blas.handle};
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
auto Primitive::isRayTraced() const -> bool { return isRayTraced_; }

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
  buildAS(idx);
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
void Primitive::buildAS(uint32_t idx) {
  auto &frame = frames[idx];
  auto scratchBuffer = allocBuildScratchBuffer(scene.device, *frame.blas.as);
  auto scratchBufferInfo = scratchBuffer->bufferInfo();
  vk::AccelerationStructureInfoNV info{
    frame.blas.type, frame.blas.flags, 0, 1, &frame.blas.geometry};

  vk::QueryPoolCreateInfo qpci{};
  qpci.queryCount = 1;
  qpci.queryType = vk::QueryType::eAccelerationStructureCompactedSizeNV;
  auto vkDevice = scene.device.vkDevice();
  auto queryPool = vkDevice.createQueryPoolUnique(qpci);
  scene.device.execSync(
    [&](vk::CommandBuffer cb) {
      cb.buildAccelerationStructureNV(
        info, nullptr, 0, false, *frame.blas.as, nullptr, scratchBufferInfo.buffer,
        scratchBufferInfo.offset);
      cb.writeAccelerationStructuresPropertiesNV(
        *frame.blas.as, vk::QueryType::eAccelerationStructureCompactedSizeNV, *queryPool,
        0);
    },
    idx);
  // Get the size result back
  vk::DeviceSize compactSize{0};
  vkDevice.getQueryPoolResults<vk::DeviceSize>(
    *queryPool, 0, 1, compactSize, sizeof(vk::DeviceSize),
    vk::QueryResultFlagBits::eWait);

  if(compactSize > 0) {
    auto originalASDesc = std::move(frame.blas);
    auto originalSize = static_cast<uint32_t>(originalASDesc.buffer->size());
    debugLog("compact from ", originalSize, " to ", compactSize);
    //do compaction
    scene.device.execSync(
      [&](vk::CommandBuffer cb) {
        // Compacting
        allocAS(scene.device, frame.blas, 0, 0, nullptr, compactSize);
        cb.copyAccelerationStructureNV(
          *frame.blas.as, *originalASDesc.as,
          vk::CopyAccelerationStructureModeNV::eCompact);
      },
      idx);
  }
}
}
