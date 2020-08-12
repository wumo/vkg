#include "device.hpp"
#include <string>
#include <set>
#include <queue>
#include "vkg/util/syntactic_sugar.hpp"

namespace vkg {

void executeImmediately(
  const vk::Device &device, const vk::CommandPool cmdPool, const vk::Queue queue,
  const std::function<void(vk::CommandBuffer)> &func, uint64_t timeout) {
  vk::CommandBufferAllocateInfo cmdBufferInfo{
    cmdPool, vk::CommandBufferLevel::ePrimary, 1};
  auto cmdBuffers = device.allocateCommandBuffers(cmdBufferInfo);
  cmdBuffers[0].begin(
    vk::CommandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  func(cmdBuffers[0]);
  cmdBuffers[0].end();

  vk::SubmitInfo submit;
  submit.commandBufferCount = uint32_t(cmdBuffers.size());
  submit.pCommandBuffers = cmdBuffers.data();
  auto fence = device.createFenceUnique(vk::FenceCreateInfo{});
  queue.submit(submit, *fence);
  device.waitForFences(*fence, VK_TRUE, timeout);

  device.freeCommandBuffers(cmdPool, cmdBuffers);
}

auto physicalDeviceTypeString(vk::PhysicalDeviceType type) -> std::string {
  switch(type) {
    case vk::PhysicalDeviceType::eIntegratedGpu: return "Integrated GPU";
    case vk::PhysicalDeviceType::eDiscreteGpu: return "Discrete GPU";
    case vk::PhysicalDeviceType::eVirtualGpu: return "Virtual GPU";
    case vk::PhysicalDeviceType::eCpu: return "CPU";
    default: break;
  }
  return "Other";
}

auto physicalDevicePreference(vk::PhysicalDeviceType type) -> int {
  switch(type) {
    case vk::PhysicalDeviceType::eDiscreteGpu: return 4;
    case vk::PhysicalDeviceType::eIntegratedGpu: return 3;
    case vk::PhysicalDeviceType::eCpu: return 2;
    case vk::PhysicalDeviceType::eVirtualGpu: return 1;
    default: break;
  }
  return 0;
}

auto chooseBestPerformantGPU(std::vector<vk::PhysicalDevice> &physicalDevices)
  -> vk::PhysicalDevice {
  errorIf(physicalDevices.empty(), "failed to find GPUs with Vulkan support!");

  auto cmp = [](vk::PhysicalDevice a, vk::PhysicalDevice b) {
    auto pA = physicalDevicePreference(a.getProperties().deviceType);
    auto pB = physicalDevicePreference(b.getProperties().deviceType);
    return pA < pB;
  };
  std::priority_queue<vk::PhysicalDevice, std::vector<vk::PhysicalDevice>, decltype(cmp)>
    queue(cmp);
  for(auto &d: physicalDevices)
    queue.push(d);
  auto chosen = queue.top();
  auto prop = chosen.getProperties();
  debugLog(
    "Chosen Device: ", prop.deviceName, ", ", physicalDeviceTypeString(prop.deviceType),
    ", API: ", prop.apiVersion >> 22u, ".", (prop.apiVersion >> 12u) & 0x3ffu, ".",
    prop.apiVersion & 0xfffu);
  return chosen;
}

void Device::findQueueFamily(FeatureConfig featureConfig) {
  auto queueFamilies = physicalDevice_.getQueueFamilyProperties();
  using Flag = vk::QueueFlagBits;

  auto search = [&](vk::QueueFlagBits queueFlag, bool required) {
    auto best = VK_QUEUE_FAMILY_IGNORED;
    for(uint32_t i = 0; i < queueFamilies.size(); i++) {
      auto flag = queueFamilies[i].queueFlags;
      if(!(flag & queueFlag)) continue;
      if(best == VK_QUEUE_FAMILY_IGNORED) best = i;
      switch(queueFlag) {
        case vk::QueueFlagBits::eGraphics: return i;
        case vk::QueueFlagBits::eCompute:
          // Dedicated queue for compute
          // Try to find a queue family index that supports compute but not graphics_
          if(!(flag & Flag::eGraphics)) return i;
          break;
        case vk::QueueFlagBits::eTransfer:
          // Dedicated queue for transfer
          // Try to find a queue family index that supports transfer but not graphics_ and compute
          if(!(flag & Flag::eGraphics) && !(flag & Flag::eCompute)) return i;
          break;
        case vk::QueueFlagBits::eSparseBinding:
        case vk::QueueFlagBits::eProtected: return i;
      }
    }
    errorIf(
      required && best == VK_QUEUE_FAMILY_IGNORED,
      "failed to find required queue family");
    return best;
  };
  graphics_ = {search(Flag::eGraphics, true)};
  compute_ = {search(Flag::eCompute, true)};
  transfer_ = {search(Flag::eTransfer, true)};
  {
    for(uint32_t i = 0; i < queueFamilies.size(); i++)
      if(physicalDevice_.getSurfaceSupportKHR(i, surface)) {
        present_.index = i;
        break;
      }
    errorIf(present_.index == VK_QUEUE_FAMILY_IGNORED, "failed to find present family!");
  }

  debugLog(
    "Queue Family: ", "graphics[", graphics_.index, "] ", "compute[", compute_.index,
    "] ", "transfer[", transfer_.index, "] ", "present[", present_.index, "]");
}

void checkDeviceExtensionSupport(
  const vk::PhysicalDevice &device, std::vector<const char *> &requiredExtensions) {
  std::set<std::string> availableExtensions;
  for(auto extension: device.enumerateDeviceExtensionProperties())
    availableExtensions.insert(std::string{(const char *)extension.extensionName});
  for(const auto *required: requiredExtensions) {
    errorIf(
      !availableExtensions.contains(required), "required device extension:", required,
      "is not supported!");
    debugLog(required);
  }
}

Device::Device(Instance &instance, vk::SurfaceKHR surface, FeatureConfig featureConfig)
  : instance{instance}, surface{surface}, featureConfig_{featureConfig} {
  auto physicalDevices = instance.vkInstance().enumeratePhysicalDevices();
  physicalDevice_ = chooseBestPerformantGPU(physicalDevices);

  findQueueFamily(featureConfig);

  limits_ = physicalDevice_.getProperties().limits;
  memProps_ = physicalDevice_.getMemoryProperties();

  std::vector<const char *> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  vk::PhysicalDeviceScalarBlockLayoutFeaturesEXT scalarBlockLayoutFeature;
  vk::PhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeature;
  vk::PhysicalDeviceMultiviewFeatures multiviewFeatures;
  vk::PhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures;

  auto features2 = physicalDevice_.getFeatures2();
  void **pNext = &features2.pNext;

  auto features12 =
    physicalDevice_
      .getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features>()
      .get<vk::PhysicalDeviceVulkan12Features>();

  *pNext = &features12;
  pNext = &features12.pNext;

  errorIf(
    !features12.scalarBlockLayout, "required feature scalarBlockLayout not supported!");
  errorIf(
    !features12.drawIndirectCount, "required feature drawIndirectCount not supported!");
  errorIf(
    !features12.timelineSemaphore, "required feature timelineSemaphore not supported!");

#if defined(USE_DEBUG_PRINTF)
  deviceExtensions.push_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
  errorIf(
    !features.vertexPipelineStoresAndAtomics || !features.fragmentStoresAndAtomics,
    "debug printf requires vertexPipelineStoresAndAtomics and "
    "fragmentStoresAndAtomics.");
#endif
  deviceExtensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
  deviceExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);

  if(featureConfig.rayTracing) {
    deviceExtensions.push_back(VK_NV_RAY_TRACING_EXTENSION_NAME);
    if(instance.supported().externalSync) {
      append(
        deviceExtensions, {
                            VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
                            VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
                            VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME,
                          });
#ifdef WIN32
      deviceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
      deviceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
      deviceExtensions.push_back(VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME);
#else
      deviceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
      deviceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
      deviceExtensions.push_back(VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME);
#endif
    }
    auto result = physicalDevice_.getProperties2<
      vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPropertiesNV>();
    rayTracingProperties_ = result.get<vk::PhysicalDeviceRayTracingPropertiesNV>();
  }

  checkDeviceExtensionSupport(physicalDevice_, deviceExtensions);

  std::set<uint32_t> indices{};

  indices.insert(graphics_.index);
  indices.insert(present_.index);
  indices.insert(compute_.index);
  indices.insert(transfer_.index);

  float priorities[] = {0.0f};
  std::vector<vk::DeviceQueueCreateInfo> queueInfos;
  queueInfos.reserve(indices.size());
  for(auto index: indices)
    queueInfos.emplace_back(vk::DeviceQueueCreateFlags{}, index, 1, priorities);

  vk::DeviceCreateInfo deviceInfo;
  deviceInfo.pNext = &features2;
  deviceInfo.queueCreateInfoCount = uint32_t(queueInfos.size());
  deviceInfo.pQueueCreateInfos = queueInfos.data();
  deviceInfo.enabledExtensionCount = uint32_t(deviceExtensions.size());
  deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
  deviceInfo.pEnabledFeatures = nullptr;

  device_ = physicalDevice_.createDeviceUnique(deviceInfo);

  {
    graphics_.queue = device_->getQueue(graphics_.index, 0);
    graphicsCmdPool_ = device_->createCommandPoolUnique(vk::CommandPoolCreateInfo{
      {vk::CommandPoolCreateFlagBits::eResetCommandBuffer}, graphics_.index});
  }
  {
    present_.queue = device_->getQueue(present_.index, 0);
    presentCmdPool_ = device_->createCommandPoolUnique(vk::CommandPoolCreateInfo{
      {vk::CommandPoolCreateFlagBits::eResetCommandBuffer}, present_.index});
  }
  {
    compute_.queue = device_->getQueue(compute_.index, 0);
    computeCmdPool_ = device_->createCommandPoolUnique(vk::CommandPoolCreateInfo{
      {vk::CommandPoolCreateFlagBits::eResetCommandBuffer}, compute_.index});
  }
  {
    transfer_.queue = device_->getQueue(transfer_.index, 0);
    transferCmdPool_ = device_->createCommandPoolUnique(vk::CommandPoolCreateInfo{
      {vk::CommandPoolCreateFlagBits::eResetCommandBuffer}, transfer_.index});
  }

  VULKAN_HPP_DEFAULT_DISPATCHER.init(*device_);

  createAllocator();
}

void Device::createAllocator() {
  VmaAllocatorCreateInfo createInfo{};
  createInfo.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
  createInfo.physicalDevice = physicalDevice_;
  createInfo.device = *device_;
  auto &dispatcher = VULKAN_HPP_DEFAULT_DISPATCHER;
  vulkanFunctions = {
    dispatcher.vkGetPhysicalDeviceProperties,
    dispatcher.vkGetPhysicalDeviceMemoryProperties,
    dispatcher.vkAllocateMemory,
    dispatcher.vkFreeMemory,
    dispatcher.vkMapMemory,
    dispatcher.vkUnmapMemory,
    dispatcher.vkFlushMappedMemoryRanges,
    dispatcher.vkInvalidateMappedMemoryRanges,
    dispatcher.vkBindBufferMemory,
    dispatcher.vkBindImageMemory,
    dispatcher.vkGetBufferMemoryRequirements,
    dispatcher.vkGetImageMemoryRequirements,
    dispatcher.vkCreateBuffer,
    dispatcher.vkDestroyBuffer,
    dispatcher.vkCreateImage,
    dispatcher.vkDestroyImage,
    dispatcher.vkCmdCopyBuffer,
    dispatcher.vkGetBufferMemoryRequirements2KHR,
    dispatcher.vkGetImageMemoryRequirements2KHR,
    dispatcher.vkBindBufferMemory2KHR,
    dispatcher.vkBindImageMemory2KHR,
    dispatcher.vkGetPhysicalDeviceMemoryProperties2KHR};
  createInfo.pVulkanFunctions = &vulkanFunctions;
  allocator_ = UniqueAllocator(new VmaAllocator(), [](VmaAllocator *ptr) {
    vmaDestroyAllocator(*ptr);
    delete ptr;
  });
  auto result = vmaCreateAllocator(&createInfo, allocator_.get());
  errorIf(result != VK_SUCCESS, "failed to create Allocator");
}

void Device::execSyncInGraphicsQueue(
  const std::function<void(vk::CommandBuffer cb)> &func, uint64_t timeout) {
  executeImmediately(*device_, *graphicsCmdPool_, graphics_.queue, func, timeout);
}
void Device::execSyncInComputeQueue(
  const std::function<void(vk::CommandBuffer)> &func, uint64_t timeout) {
  executeImmediately(*device_, *computeCmdPool_, compute_.queue, func, timeout);
}

auto Device::physicalDevice() -> vk::PhysicalDevice { return physicalDevice_; }
auto Device::memProps() -> const vk::PhysicalDeviceMemoryProperties & {
  return memProps_;
}
auto Device::limits() -> const vk::PhysicalDeviceLimits & { return limits_; }
auto Device::vkDevice() -> vk::Device { return *device_; }
Device::operator vk::Device() { return *device_; }
auto Device::allocator() -> VmaAllocator { return *allocator_; }
auto Device::rayTracingProperties() -> const vk::PhysicalDeviceRayTracingPropertiesNV & {
  return rayTracingProperties_;
}
auto Device::graphicsCmdPool() -> vk::CommandPool { return *graphicsCmdPool_; }
auto Device::graphicsQueue() const -> vk::Queue { return graphics_.queue; }
auto Device::graphicsIndex() const -> uint32_t { return graphics_.index; }
auto Device::presentCmdPool() -> vk::CommandPool { return *presentCmdPool_; }
auto Device::presentIndex() const -> uint32_t { return present_.index; }
auto Device::presentQueue() const -> vk::Queue { return present_.queue; }
auto Device::computeCmdPool() -> vk::CommandPool { return *computeCmdPool_; }
auto Device::computeIndex() const -> uint32_t { return compute_.index; }
auto Device::computeQueue() const -> vk::Queue { return compute_.queue; }
auto Device::transferCmdPool() -> vk::CommandPool { return *transferCmdPool_; }
auto Device::transferIndex() const -> uint32_t { return transfer_.index; }
auto Device::transferQueue() const -> vk::Queue { return transfer_.queue; }

auto Device::multiviewProperties() -> const vk::PhysicalDeviceMultiviewProperties & {
  return multiviewProperties_;
}

}