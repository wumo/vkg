#include "acceleration_structures.hpp"
namespace vkg {

void allocAS(
  Device &device, ASDesc &asDesc, vk::AccelerationStructureTypeNV type,
  const vk::BuildAccelerationStructureFlagsNV &flags, uint32_t instanceCount,
  uint32_t geometryCount, const vk::GeometryNV *geometries,
  vk::DeviceSize compactedSize) {
  asDesc.info = {type, flags, instanceCount, geometryCount, geometries};
  asDesc.as = device.vkDevice().createAccelerationStructureNVUnique(
    {compactedSize, asDesc.info}, nullptr);

  vk::AccelerationStructureMemoryRequirementsInfoNV memReqInfo{
    vk::AccelerationStructureMemoryRequirementsTypeNV::eObject, *asDesc.as};
  auto memReq =
    device.vkDevice().getAccelerationStructureMemoryRequirementsNV(memReqInfo);

  asDesc.buffer = buffer::devRayTracingBuffer(
    device, memReq.memoryRequirements.size, memReq.memoryRequirements.memoryTypeBits);
  auto [mem, offset] = asDesc.buffer->devMem();
  vk::BindAccelerationStructureMemoryInfoNV bindInfo{*asDesc.as, mem, offset};
  device.vkDevice().bindAccelerationStructureMemoryNV(bindInfo);

  device.vkDevice().getAccelerationStructureHandleNV<uint64_t>(*asDesc.as, asDesc.handle);
}
}