#pragma once
#include "buffers.hpp"

namespace vkg {
struct ASDesc {
  std::unique_ptr<Buffer> asBuffer;
  vk::UniqueAccelerationStructureNV as;
  uint64_t handle{0};

  vk::AccelerationStructureTypeNV type{};
  vk::BuildAccelerationStructureFlagsNV flags{};
  uint32_t geometryCount{0};
  std::unique_ptr<Buffer> scratchBuffer;
};

void allocAS(
  Device &device, ASDesc &asDesc, uint32_t instanceCount, uint32_t geometryCount,
  const vk::GeometryNV *geometries, vk::DeviceSize compactedSize = 0);

auto allocBuildScratchBuffer(
  Device &device, vk::AccelerationStructureNV as,
  const std::string &name = "BuildScratchBuffer") -> std::unique_ptr<Buffer>;
auto buildScratchBufferSize(Device &device, vk::AccelerationStructureNV as) -> uint32_t;
auto allocUpdateScratchBuffer(
  Device &device, vk::AccelerationStructureNV as,
  const std::string &name = "UpdateScratchBuffer") -> std::unique_ptr<Buffer>;
auto updateScratchBufferSize(Device &device, vk::AccelerationStructureNV as) -> uint32_t;
}
