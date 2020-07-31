#pragma once
#include "vk_headers.hpp"
#include "config.hpp"
#include <tuple>
#include <functional>
#include "instance.hpp"

namespace vkg {
struct QueueInfo {
  /**queue family index*/
  uint32_t index{VK_QUEUE_FAMILY_IGNORED};
  vk::Queue queue;
};

class Device {
public:
  struct SupportedExtension {
    bool multiview{false};
    bool descriptorIndexing{false};
    bool externalSync{false};
  };

  Device(Instance &instance, vk::SurfaceKHR surface, FeatureConfig featureConfig = {});

  void execSyncInGraphicsQueue(
    const std::function<void(vk::CommandBuffer cb)> &func,
    uint64_t timeout = std::numeric_limits<uint64_t>::max());
  void execSyncInComputeQueue(
    const std::function<void(vk::CommandBuffer cb)> &func,
    uint64_t timeout = std::numeric_limits<uint64_t>::max());

  auto physicalDevice() -> vk::PhysicalDevice;
  auto memProps() -> const vk::PhysicalDeviceMemoryProperties &;
  auto limits() -> const vk::PhysicalDeviceLimits &;
  operator vk::Device();
  auto vkDevice() -> vk::Device;
  auto allocator() -> VmaAllocator;
  auto graphicsCmdPool() -> vk::CommandPool;
  auto graphicsIndex() const -> uint32_t;
  auto graphicsQueue() const -> vk::Queue;
  auto presentCmdPool() -> vk::CommandPool;
  auto presentIndex() const -> uint32_t;
  auto presentQueue() const -> vk::Queue;
  auto computeCmdPool() -> vk::CommandPool;
  auto computeIndex() const -> uint32_t;
  auto computeQueue() const -> vk::Queue;
  auto transferCmdPool() -> vk::CommandPool;
  auto transferIndex() const -> uint32_t;
  auto transferQueue() const -> vk::Queue;
  auto rayTracingProperties() -> const vk::PhysicalDeviceRayTracingPropertiesNV &;
  auto multiviewProperties() -> const vk::PhysicalDeviceMultiviewProperties &;

  void name(vk::Buffer object, const std::string &markerName);
  void name(vk::Image object, const std::string &markerName);
  void name(vk::ImageView object, const std::string &markerName);
  void name(vk::Pipeline object, const std::string &markerName);
  void name(vk::DescriptorSet object, const std::string &markerName);
  void name(
    uint64_t object, vk::DebugReportObjectTypeEXT objectType,
    const std::string &markerName);
  void tag(
    uint64_t object, vk::DebugReportObjectTypeEXT objectType, uint64_t name,
    size_t tagSize, const void *tag);
  void begin(
    vk::CommandBuffer commandBuffer, const std::string &markerName,
    std::array<float, 4> color = {1, 1, 1, 1});
  void insert(
    vk::CommandBuffer commandBuffer, const std::string &markerName,
    std::array<float, 4> color = {1, 1, 1, 1});
  void end(vk::CommandBuffer commandBuffer);

private:
  FeatureConfig featureConfig_;
  Instance &instance;
  vk::SurfaceKHR surface;
  vk::PhysicalDevice physicalDevice_;
  vk::PhysicalDeviceMemoryProperties memProps_;
  vk::PhysicalDeviceLimits limits_;
  vk::UniqueDevice device_;

  VmaVulkanFunctions vulkanFunctions{};
  using UniqueAllocator =
    std::unique_ptr<VmaAllocator, std::function<void(VmaAllocator *)>>;
  UniqueAllocator allocator_;

  vk::PhysicalDeviceFeatures features;
  vk::PhysicalDeviceFeatures2 features2;

  SupportedExtension supported_;

  vk::PhysicalDeviceRayTracingPropertiesNV rayTracingProperties_;
  vk::PhysicalDeviceMultiviewProperties multiviewProperties_;

  vk::UniqueCommandPool graphicsCmdPool_;
  QueueInfo graphics_;
  vk::UniqueCommandPool presentCmdPool_;
  QueueInfo present_;
  vk::UniqueCommandPool computeCmdPool_;
  QueueInfo compute_;
  vk::UniqueCommandPool transferCmdPool_;
  QueueInfo transfer_;

private:
  void findQueueFamily(FeatureConfig featureConfig);
  void createAllocator();
};
}
