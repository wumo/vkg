#pragma once
#include "buffer.hpp"

namespace vkg::buffer {
auto devBuffer(
    Device &device, const vk::BufferUsageFlags &usage, vk::DeviceSize sizeInBytes,
    const std::string &name = "devBuffer") -> std::unique_ptr<Buffer>;

auto hostCoherentBuffer(
    Device &device, const vk::BufferUsageFlags &usage, vk::DeviceSize sizeInBytes, VmaMemoryUsage memoryUsage,
    const std::string &name = "hostCoherentBuffer") -> std::unique_ptr<Buffer>;
auto hostOnlyBuffer(
    Device &device, const vk::BufferUsageFlags &usage, vk::DeviceSize sizeInBytes,
    const std::string &name = "hostOnlyBuffer") -> std::unique_ptr<Buffer>;
auto readbackBuffer(
    Device &device, const vk::BufferUsageFlags &usage, vk::DeviceSize sizeInBytes,
    const std::string &name = "readbackBuffer") -> std::unique_ptr<Buffer>;
auto hostBuffer(
    Device &device, const vk::BufferUsageFlags &usage, vk::DeviceSize sizeInBytes,
    const std::string &name = "hostBuffer") -> std::unique_ptr<Buffer>;

auto devIndexBuffer(Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "devIndexBuffer")
    -> std::unique_ptr<Buffer>;
auto hostIndexBuffer(Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostIndexBuffer")
    -> std::unique_ptr<Buffer>;
auto hostOnlyIndexBuffer(Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostOnlyIndexBuffer")
    -> std::unique_ptr<Buffer>;
auto devIndexStorageBuffer(
    Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "devIndexStorageBuffer")
    -> std::unique_ptr<Buffer>;
auto hostIndexStorageBuffer(
    Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostIndexStorageBuffer")
    -> std::unique_ptr<Buffer>;
auto hostOnlyIndexStorageBuffer(
    Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostOnlyIndexStorageBuffer")
    -> std::unique_ptr<Buffer>;

auto devVertexBuffer(Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "devVertexBuffer")
    -> std::unique_ptr<Buffer>;
auto hostVertexBuffer(Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostVertexBuffer")
    -> std::unique_ptr<Buffer>;
auto hostOnlyVertexBuffer(Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostOnlyVertexBuffer")
    -> std::unique_ptr<Buffer>;
auto devVertexStorageBuffer(
    Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "devVertexStorageBuffer")
    -> std::unique_ptr<Buffer>;
auto hostVertexStorageBuffer(
    Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostVertexStorageBuffer")
    -> std::unique_ptr<Buffer>;
auto hostOnlyVertexStorageBuffer(
    Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostOnlyVertexStorageBuffer")
    -> std::unique_ptr<Buffer>;

auto devUniformBuffer(Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "devUniformBuffer")
    -> std::unique_ptr<Buffer>;
auto hostUniformBuffer(Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostUniformBuffer")
    -> std::unique_ptr<Buffer>;
auto hostOnlyUniformBuffer(
    Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostOnlyUniformBuffer")
    -> std::unique_ptr<Buffer>;

auto devStorageBuffer(Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "devStorageBuffer")
    -> std::unique_ptr<Buffer>;
auto hostStorageBuffer(Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostStorageBuffer")
    -> std::unique_ptr<Buffer>;
auto hostOnlyStorageBuffer(
    Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostOnlyStorageBuffer")
    -> std::unique_ptr<Buffer>;

auto devIndirectBuffer(Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "devIndirectBuffer")
    -> std::unique_ptr<Buffer>;
auto devIndirectStorageBuffer(
    Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "devIndirectStorageBuffer")
    -> std::unique_ptr<Buffer>;
auto hostIndirectBuffer(Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostIndirectBuffer")
    -> std::unique_ptr<Buffer>;
auto hostIndirectStorageBuffer(
    Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostIndirectStorageBuffer")
    -> std::unique_ptr<Buffer>;
auto hostOnlyIndirectBuffer(
    Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostOnlyIndirectBuffer")
    -> std::unique_ptr<Buffer>;

auto devRayTracingBuffer(
    Device &device, vk::DeviceSize sizeInBytes, uint32_t memoryTypeBits,
    const std::string &name = "devRayTracingBuffer") -> std::unique_ptr<Buffer>;
auto devRayTracingBuffer2(Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "devRayTracingBuffer")
    -> std::unique_ptr<Buffer>;
auto hostRayTracingBuffer(Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostRayTracingBuffer")
    -> std::unique_ptr<Buffer>;
auto hostOnlyRayTracingBuffer(
    Device &device, vk::DeviceSize sizeInBytes, const std::string &name = "hostOnlyRayTracingBuffer")
    -> std::unique_ptr<Buffer>;

void upload(
    uint32_t queueIdx, Buffer &buffer, const void *value, vk::DeviceSize sizeInBytes,
    vk::DeviceSize dstOffsetInBytes = 0);

template<class Type>
auto uploadSingle(uint32_t queueIdx, Buffer &buffer, Type &value, vk::DeviceSize dstOffsetInBytes = 0) -> void {
    upload(queueIdx, buffer, &value, sizeof(value), dstOffsetInBytes);
}

template<class Type, class Allocator>
auto uploadVec(
    uint32_t queueIdx, Buffer &buffer, const std::vector<Type, Allocator> &value, vk::DeviceSize dstOffsetInBytes = 0)
    -> void {
    upload(queueIdx, buffer, value.data(), value.size() * sizeof(Type), dstOffsetInBytes);
}

void updateBytes(Buffer &buffer, const void *value, vk::DeviceSize sizeInBytes, vk::DeviceSize dstOffsetInBytes = 0);

template<class Type>
void updateSingle(Buffer &buffer, Type &value, vk::DeviceSize dstOffsetInBytes = 0) {
    updateBytes(buffer, &value, sizeof(value), dstOffsetInBytes);
}
template<class Type, class Allocator>
void updateVec(Buffer &buffer, const std::vector<Type, Allocator> &value, vk::DeviceSize dstOffsetInBytes = 0) {
    updateBytes(buffer, value.data(), value.size() * sizeof(Type), dstOffsetInBytes);
}
}
