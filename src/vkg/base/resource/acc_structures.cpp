#include "acc_structures.hpp"
namespace vkg {

void allocAS(
  Device &device, ASDesc &asDesc, uint32_t instanceCount, uint32_t geometryCount,
  const vk::GeometryNV *geometries, vk::DeviceSize compactedSize) {
  auto dev = device.vkDevice();
  vk::AccelerationStructureInfoNV info{
    asDesc.type, asDesc.flags, instanceCount, geometryCount, geometries};
  asDesc.as = dev.createAccelerationStructureNVUnique({compactedSize, info}, nullptr);

  vk::AccelerationStructureMemoryRequirementsInfoNV memReqInfo{
    vk::AccelerationStructureMemoryRequirementsTypeNV::eObject, *asDesc.as};
  auto memReq = dev.getAccelerationStructureMemoryRequirementsNV(memReqInfo);

  asDesc.buffer = buffer::devRayTracingBuffer(
    device, memReq.memoryRequirements.size, memReq.memoryRequirements.memoryTypeBits);
  auto [mem, offset] = asDesc.buffer->devMem();
  vk::BindAccelerationStructureMemoryInfoNV bindInfo{*asDesc.as, mem, offset};
  dev.bindAccelerationStructureMemoryNV(bindInfo);

  dev.getAccelerationStructureHandleNV<uint64_t>(*asDesc.as, asDesc.handle);
}
auto allocBuildScratchBuffer(Device &device, vk::AccelerationStructureNV as)
  -> std::unique_ptr<Buffer> {
  vk::AccelerationStructureMemoryRequirementsInfoNV memReqInfo{
    vk::AccelerationStructureMemoryRequirementsTypeNV::eBuildScratch, as};
  auto memReq =
    device.vkDevice().getAccelerationStructureMemoryRequirementsNV(memReqInfo);
  return buffer::devRayTracingBuffer(
    device, memReq.memoryRequirements.size, memReq.memoryRequirements.memoryTypeBits);
}
auto allocUpdateScratchBuffer(Device &device, vk::AccelerationStructureNV as)
  -> std::unique_ptr<Buffer> {
  vk::AccelerationStructureMemoryRequirementsInfoNV memReqInfo{
    vk::AccelerationStructureMemoryRequirementsTypeNV::eUpdateScratch, as};
  auto memReq =
    device.vkDevice().getAccelerationStructureMemoryRequirementsNV(memReqInfo);
  return buffer::devRayTracingBuffer(
    device, memReq.memoryRequirements.size, memReq.memoryRequirements.memoryTypeBits);
}
}