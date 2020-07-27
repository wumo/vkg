#include "buffers.hpp"

namespace vkg::buffer {
auto devBuffer(
  Device &device, const vk::BufferUsageFlags &usage, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  vk::SharingMode sharingMode = vk::SharingMode::eExclusive;
  uint32_t queueFamilyIndexCount = 0;
  uint32_t queueFamilyIndices[]{device.graphicsIndex(), device.computeIndex()};
  if(device.graphicsIndex() != device.computeIndex()) {
    sharingMode = vk::SharingMode::eConcurrent;
    queueFamilyIndexCount = 2;
  }
  vk::BufferCreateInfo info{
    {},          sizeInBytes,           usage | vk::BufferUsageFlagBits::eTransferDst,
    sharingMode, queueFamilyIndexCount, queueFamilyIndices};
  VmaAllocationCreateInfo allocInfo{{}, VMA_MEMORY_USAGE_GPU_ONLY};
  auto buffer = std::make_unique<Buffer>(device, info, allocInfo);
  return buffer;
}

auto hostCoherentBuffer(
  Device &device, const vk::BufferUsageFlags &usage, vk::DeviceSize sizeInBytes,
  VmaMemoryUsage memoryUsage) -> std::unique_ptr<Buffer> {
  vk::SharingMode sharingMode = vk::SharingMode::eExclusive;
  uint32_t queueFamilyIndexCount = 0;
  uint32_t queueFamilyIndices[]{device.graphicsIndex(), device.computeIndex()};
  if(device.graphicsIndex() != device.computeIndex()) {
    sharingMode = vk::SharingMode::eConcurrent;
    queueFamilyIndexCount = 2;
  }
  vk::BufferCreateInfo info{
    {},          sizeInBytes,           usage | vk::BufferUsageFlagBits::eTransferSrc,
    sharingMode, queueFamilyIndexCount, queueFamilyIndices};
  VmaAllocationCreateInfo allocInfo{
    VMA_ALLOCATION_CREATE_MAPPED_BIT, memoryUsage, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
  auto buffer = std::make_unique<Buffer>(device, info, allocInfo);
  return buffer;
}

auto hostOnlyBuffer(
  Device &device, const vk::BufferUsageFlags &usage, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostCoherentBuffer(device, usage, sizeInBytes, VMA_MEMORY_USAGE_CPU_ONLY);
}

auto readbackBuffer(
  Device &device, const vk::BufferUsageFlags &usage, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostCoherentBuffer(device, usage, sizeInBytes, VMA_MEMORY_USAGE_GPU_TO_CPU);
}

auto hostBuffer(
  Device &device, const vk::BufferUsageFlags &usage, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostCoherentBuffer(device, usage, sizeInBytes, VMA_MEMORY_USAGE_CPU_TO_GPU);
}

auto devIndexBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return devBuffer(device, vk::BufferUsageFlagBits::eIndexBuffer, sizeInBytes);
}
auto hostIndexBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostBuffer(device, vk::BufferUsageFlagBits::eIndexBuffer, sizeInBytes);
}
auto hostOnlyIndexBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostOnlyBuffer(device, vk::BufferUsageFlagBits::eIndexBuffer, sizeInBytes);
}
auto devIndexStorageBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return devBuffer(
    device,
    vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
    sizeInBytes);
}
auto hostIndexStorageBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostBuffer(
    device,
    vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
    sizeInBytes);
}
auto hostOnlyIndexStorageBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostOnlyBuffer(
    device,
    vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
    sizeInBytes);
}

auto devVertexBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return devBuffer(device, vk::BufferUsageFlagBits::eVertexBuffer, sizeInBytes);
}
auto hostVertexBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostBuffer(device, vk::BufferUsageFlagBits::eVertexBuffer, sizeInBytes);
}
auto hostOnlyVertexBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostOnlyBuffer(device, vk::BufferUsageFlagBits::eVertexBuffer, sizeInBytes);
}
auto devVertexStorageBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return devBuffer(
    device,
    vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
    sizeInBytes);
}
auto hostVertexStorageBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostBuffer(
    device,
    vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
    sizeInBytes);
}
auto hostOnlyVertexStorageBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostOnlyBuffer(
    device,
    vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
    sizeInBytes);
}

auto devUniformBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return devBuffer(device, vk::BufferUsageFlagBits::eUniformBuffer, sizeInBytes);
}
auto hostUniformBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostBuffer(device, vk::BufferUsageFlagBits::eUniformBuffer, sizeInBytes);
}
auto hostOnlyUniformBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostOnlyBuffer(device, vk::BufferUsageFlagBits::eUniformBuffer, sizeInBytes);
}

auto devStorageBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return devBuffer(device, vk::BufferUsageFlagBits::eStorageBuffer, sizeInBytes);
}
auto hostStorageBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostBuffer(device, vk::BufferUsageFlagBits::eStorageBuffer, sizeInBytes);
}
auto hostOnlyStorageBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostOnlyBuffer(device, vk::BufferUsageFlagBits::eStorageBuffer, sizeInBytes);
}

auto devIndirectBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return devBuffer(device, vk::BufferUsageFlagBits::eIndirectBuffer, sizeInBytes);
}
auto devIndirectStorageBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return devBuffer(
    device,
    vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
    sizeInBytes);
}
auto hostIndirectBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostBuffer(device, vk::BufferUsageFlagBits::eIndirectBuffer, sizeInBytes);
}
auto hostIndirectStorageBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostBuffer(
    device,
    vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
    sizeInBytes);
}
auto hostOnlyIndirectBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostOnlyBuffer(device, vk::BufferUsageFlagBits::eIndirectBuffer, sizeInBytes);
}

auto devRayTracingBuffer(
  Device &device, vk::DeviceSize sizeInBytes, uint32_t memoryTypeBits)
  -> std::unique_ptr<Buffer> {
  vk::BufferCreateInfo info{{}, sizeInBytes, vk::BufferUsageFlagBits::eRayTracingNV};
  VmaAllocationCreateInfo allocInfo{};
  allocInfo.memoryTypeBits = memoryTypeBits;
  auto buffer = std::make_unique<Buffer>(device, info, allocInfo);
  return buffer;
}

auto devRayTracingBuffer2(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return devBuffer(device, vk::BufferUsageFlagBits::eRayTracingNV, sizeInBytes);
}

auto hostRayTracingBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostBuffer(device, vk::BufferUsageFlagBits::eRayTracingNV, sizeInBytes);
}
auto hostOnlyRayTracingBuffer(Device &device, vk::DeviceSize sizeInBytes)
  -> std::unique_ptr<Buffer> {
  return hostOnlyBuffer(device, vk::BufferUsageFlagBits::eRayTracingNV, sizeInBytes);
}

void updateBytes(
  Buffer &buffer, const void *value, vk::DeviceSize sizeInBytes,
  vk::DeviceSize dstOffsetInBytes) {
  memcpy(buffer.ptr<std::byte>() + dstOffsetInBytes, value, size_t(sizeInBytes));
}

void upload(
  Buffer &buffer, const void *value, vk::DeviceSize sizeInBytes,
  vk::DeviceSize dstOffsetInBytes) {
  if(sizeInBytes == 0) return;
  auto &device = buffer.device();
  auto tmp =
    hostBuffer(buffer.device(), vk::BufferUsageFlagBits::eTransferSrc, sizeInBytes);
  updateBytes(*tmp, value, sizeInBytes);
  device.execSyncInGraphicsQueue([&](vk::CommandBuffer cb) {
    vk::BufferCopy copy{0, dstOffsetInBytes, sizeInBytes};
    cb.copyBuffer(tmp->buffer(), buffer.buffer(), copy);
  });
}

}
