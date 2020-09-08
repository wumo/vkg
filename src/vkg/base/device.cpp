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

/**
 * https://www.imgtec.com/blog/vulkan-synchronisation-and-graphics-compute-graphics-hazards-part-2/
 */
void Device::findQueueFamily() {
  auto queueFamilies = physicalDevice_.getQueueFamilyProperties();
  using Flag = vk::QueueFlagBits;

  auto desiredFlag = Flag::eGraphics | Flag::eCompute | Flag::eTransfer;

  uint32_t gfxIdx = 0;
  for(; gfxIdx < queueFamilies.size(); gfxIdx++)
    if(auto &family = queueFamilies[gfxIdx];
       physicalDevice_.getSurfaceSupportKHR(gfxIdx, surface) &&
       (family.queueFlags & desiredFlag) && family.queueCount >= queueCount) {
      break;
    }

  errorIf(
    gfxIdx >= queueFamilies.size(),
    "can't find a queue family that supports graphics/compute/transfer at the same time");
  debugLog("Queue Family: ", gfxIdx);
  queueFamily_ = gfxIdx;
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

Device::Device(
  Instance &instance, vk::SurfaceKHR surface, const FeatureConfig &featureConfig)
  : instance{instance}, surface{surface} {
  auto physicalDevices = instance.vkInstance().enumeratePhysicalDevices();
  physicalDevice_ = chooseBestPerformantGPU(physicalDevices);

  limits_ = physicalDevice_.getProperties().limits;
  memProps_ = physicalDevice_.getMemoryProperties();

  std::vector<const char *> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  auto features2 = physicalDevice_.getFeatures2();
  auto *pNext = &features2.pNext;

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

  supported_.samplerAnisotropy=features2.features.samplerAnisotropy;
#if defined(USE_DEBUG_PRINTF)
  deviceExtensions.push_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
  errorIf(
    !features.vertexPipelineStoresAndAtomics || !features.fragmentStoresAndAtomics,
    "debug printf requires vertexPipelineStoresAndAtomics and "
    "fragmentStoresAndAtomics.");
#endif
  deviceExtensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
  deviceExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);

  if(featureConfig.rayTrace) {
    append(
      deviceExtensions, {
                          VK_NV_RAY_TRACING_EXTENSION_NAME,
                          VK_KHR_MAINTENANCE3_EXTENSION_NAME,
                        });
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
    rtProperties_ =
      physicalDevice_
        .getProperties2<
          vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPropertiesNV>()
        .get<vk::PhysicalDeviceRayTracingPropertiesNV>();
  }

  checkDeviceExtensionSupport(physicalDevice_, deviceExtensions);

  queueCount = featureConfig.numFrames;

  findQueueFamily();

  std::vector<float> priorities(queueCount);

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{
    {{}, queueFamily_, queueCount, priorities.data()}};

  vk::DeviceCreateInfo deviceInfo;
  deviceInfo.pNext = &features2;
  deviceInfo.queueCreateInfoCount = uint32_t(queueCreateInfos.size());
  deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
  deviceInfo.enabledExtensionCount = uint32_t(deviceExtensions.size());
  deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
  deviceInfo.pEnabledFeatures = nullptr;

  device_ = physicalDevice_.createDeviceUnique(deviceInfo);

  queues_.resize(queueCount);
  for(uint32_t idx = 0u; idx < queueCount; ++idx)
    queues_[idx] = device_->getQueue(queueFamily_, idx);

  cmdPool_ = device_->createCommandPoolUnique(vk::CommandPoolCreateInfo{
    {vk::CommandPoolCreateFlagBits::eResetCommandBuffer}, queueFamily_});

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

void Device::execSync(
  const std::function<void(vk::CommandBuffer)> &func, uint32_t queueIdx,
  uint64_t timeout) {
  executeImmediately(*device_, *cmdPool_, queues_[queueIdx], func, timeout);
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
  return rtProperties_;
}
auto Device::multiviewProperties() -> const vk::PhysicalDeviceMultiviewProperties & {
  return multiviewProperties_;
}
auto Device::queues() -> std::span<vk::Queue> { return queues_; }
auto Device::cmdPool() -> vk::CommandPool { return *cmdPool_; }
auto Device::queueFamiliy() const -> uint32_t { return queueFamily_; }
auto Device::supported() const -> const Device::SupportedExtension & { return supported_; }

}