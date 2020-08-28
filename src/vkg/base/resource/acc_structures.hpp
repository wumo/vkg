#pragma once
#include "buffers.hpp"

namespace vkg {
struct ASDesc {
  vk::UniqueAccelerationStructureKHR as;
  std::unique_ptr<Buffer> buffer;
  uint64_t handle;

  vk::AccelerationStructureCreateGeometryTypeInfoKHR geometryInfo;
  vk::AccelerationStructureGeometryKHR geometry;
  vk::AccelerationStructureBuildOffsetInfoKHR buildOffset;
};

void allocAS(
  Device &device, ASDesc &asDesc, vk::AccelerationStructureTypeKHR type,
  const vk::BuildAccelerationStructureFlagsKHR &flags, uint32_t maxGeometryCount,
  const vk::AccelerationStructureCreateGeometryTypeInfoKHR *geometries,
  vk::DeviceSize compactedSize = 0);
}
