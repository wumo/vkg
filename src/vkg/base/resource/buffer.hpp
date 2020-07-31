#pragma once
#include "vkg/base/device.hpp"
#include <memory>

namespace vkg {
class Buffer {
public:
  Buffer(
    Device &device, vk::BufferCreateInfo info, VmaAllocationCreateInfo allocInfo,
    const std::string &name = "");

  void barrier(
    vk::CommandBuffer &cb, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask,
    vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands,
    vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands,
    vk::DependencyFlags dependencyFlags = vk::DependencyFlagBits::eByRegion,
    uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED);

  template<class T = void>
  auto ptr() -> T * {
    if(!mappable) throw std::runtime_error("this buffer is not mappable");
    return static_cast<T *>(alloc.pMappedData);
  }

  auto buffer() const -> vk::Buffer;
  auto device() const -> Device &;
  auto devMem() const -> std::pair<vk::DeviceMemory, vk::DeviceSize>;
  auto size() const -> vk::DeviceSize;

private:
  struct VmaBuffer {
    Device &vkezDevice;
    VmaAllocation allocation{nullptr};
    vk::Buffer buffer{nullptr};
  };

  using UniquePtr = std::unique_ptr<VmaBuffer, std::function<void(VmaBuffer *)>>;
  UniquePtr vmaBuffer;

  VmaAllocationInfo alloc{};
  bool mappable{false};

  vk::BufferCreateInfo info;
};
}
