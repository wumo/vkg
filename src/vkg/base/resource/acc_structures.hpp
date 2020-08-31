#pragma once
#include "buffers.hpp"

namespace vkg {
struct ASDesc {
  std::unique_ptr<Buffer> buffer;
  vk::UniqueAccelerationStructureNV as;
  uint64_t handle{0};

  vk::AccelerationStructureTypeNV type{};
  vk::BuildAccelerationStructureFlagsNV flags{};
  vk::GeometryNV geometry;
};

void allocAS(
  Device &device, ASDesc &asDesc, uint32_t instanceCount, uint32_t geometryCount,
  const vk::GeometryNV *geometries, vk::DeviceSize compactedSize = 0);

auto allocBuildScratchBuffer(Device &device, vk::AccelerationStructureNV as)
  -> std::unique_ptr<Buffer>;
auto allocUpdateScratchBuffer(Device &device, vk::AccelerationStructureNV as)
  -> std::unique_ptr<Buffer>;
}
