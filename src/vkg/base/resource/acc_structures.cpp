#include "acc_structures.hpp"
namespace vkg {

void allocAS(
  Device &device, ASDesc &asDesc, vk::AccelerationStructureTypeKHR type,
  const vk::BuildAccelerationStructureFlagsKHR &flags, uint32_t maxGeometryCount,
  const vk::AccelerationStructureCreateGeometryTypeInfoKHR *geometries,
  vk::DeviceSize compactedSize) {
  asDesc.as = device.vkDevice().createAccelerationStructureKHRUnique(
    {compactedSize, type, flags, maxGeometryCount, geometries}, nullptr);

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