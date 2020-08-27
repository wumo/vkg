#pragma once
#include "vk_headers.hpp"
#include "config.hpp"
#include <tuple>
#include <functional>
#include "instance.hpp"
#include <span>

namespace vkg {

class Device {
public:
  struct SupportedExtension {
    bool multiview{false};
    bool descriptorIndexing{false};
    bool externalSync{false};
    bool timelineSemaphore{false};
  };

  Device(Instance &instance, vk::SurfaceKHR surface, const FeatureConfig &featureConfig);

  void execSync(
    const std::function<void(vk::CommandBuffer cb)> &func, uint32_t queueIdx,
    uint64_t timeout = std::numeric_limits<uint64_t>::max());

  auto physicalDevice() -> vk::PhysicalDevice;
  auto memProps() -> const vk::PhysicalDeviceMemoryProperties &;
  auto limits() -> const vk::PhysicalDeviceLimits &;
  operator vk::Device();
  auto vkDevice() -> vk::Device;
  auto allocator() -> VmaAllocator;
  auto rayTracingProperties() -> const vk::PhysicalDeviceRayTracingPropertiesNV &;
  auto multiviewProperties() -> const vk::PhysicalDeviceMultiviewProperties &;

  auto queueFamiliy() const -> uint32_t;
  auto queues() -> std::span<vk::Queue>;
  auto cmdPool() -> vk::CommandPool;

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

  SupportedExtension supported_;

  vk::PhysicalDeviceRayTracingPropertiesNV rayTracingProperties_;
  vk::PhysicalDeviceMultiviewProperties multiviewProperties_;

  uint32_t queueFamily_{VK_QUEUE_FAMILY_IGNORED};
  uint32_t queueCount{0};
  std::vector<vk::Queue> queues_;
  vk::UniqueCommandPool cmdPool_;

private:
  void findQueueFamily();
  void createAllocator();
};
}
