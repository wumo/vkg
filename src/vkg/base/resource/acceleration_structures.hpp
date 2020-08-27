#pragma once
#include "buffers.hpp"

namespace vkg {
struct ASDesc {
  vk::AccelerationStructureInfoNV info;
  std::unique_ptr<Buffer> buffer;
  vk::UniqueAccelerationStructureNV as;
  uint64_t handle;
};

void allocAS(
  Device &device, ASDesc &asDesc, vk::AccelerationStructureTypeNV type,
  const vk::BuildAccelerationStructureFlagsNV& flags, uint32_t instanceCount,
  uint32_t geometryCount, const vk::GeometryNV *geometries,vk::DeviceSize compactedSize = 0);
}
