#include "base.hpp"
#include <iostream>
#include "vkg/util/syntactic_sugar.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace vkg {

Base::Base(const WindowConfig &windowConfig, const FeatureConfig &featureConfig)
  : featureConfig_{featureConfig} {
  window_ = std::make_unique<Window>(windowConfig);
  instance = std::make_unique<Instance>(featureConfig_);
  window_->createSurface(instance->vkInstance());
  createDebugUtils();
  device_ = std::make_unique<Device>(*instance, window_->vkSurface(), featureConfig_);
  swapchain_ =
    std::make_unique<Swapchain>(*device_, window_->vkSurface(), featureConfig_);
  createSyncObjects();
  createCommandBuffers();
  Base::resize();
}

auto Base::featureConfig() const -> const FeatureConfig & { return featureConfig_; }

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}
auto Base::createDebugUtils() -> void {
#if defined(USE_VALIDATION_LAYER)
  using severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
  using msgType = vk::DebugUtilsMessageTypeFlagBitsEXT;
  vk::DebugUtilsMessengerCreateInfoEXT createInfo;
  createInfo.messageSeverity = severity::eVerbose | severity::eWarning | severity::eError;
  createInfo.messageType = msgType::eGeneral | msgType::eValidation |
                           msgType::ePerformance;
  createInfo.pfnUserCallback = debugCallback;

  callback =
    instance->vkInstance().createDebugUtilsMessengerEXTUnique(createInfo, nullptr);
#endif
}

auto Base::createSyncObjects() -> void {
  auto _device = device_->vkDevice();
  auto numFrames = uint32_t(device_->queues().size());

  semaphoreSyncs.resize(numFrames);
  vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};
  for(uint32_t i = 0; i < numFrames; i++) {
    semaphoreSyncs[i].imageAvailable = _device.createSemaphoreUnique({});
    semaphoreSyncs[i].renderFinished = _device.createSemaphoreUnique({});

    semaphoreSyncs[i].resourcesFence = _device.createFenceUnique(fenceInfo);
  }

  timelineSyncs.resize(numFrames);
  for(auto i = 0u; i < numFrames; ++i) {
    vk::SemaphoreTypeCreateInfo timelineCreateInfo{vk::SemaphoreType::eTimeline, 0};
    vk::SemaphoreCreateInfo createInfo{};
    createInfo.pNext = &timelineCreateInfo;
    timelineSyncs[i].semaphore = _device.createSemaphoreUnique(createInfo);
    timelineSyncs[i].wsiImageAvailable = _device.createSemaphoreUnique({});
    timelineSyncs[i].wsiReadyToPresent = _device.createSemaphoreUnique({});
  }
}

auto Base::createCommandBuffers() -> void {
  auto _device = device_->vkDevice();
  auto numFrames = uint32_t(device_->queues().size());
  vk::CommandBufferAllocateInfo info{
    device_->cmdPool(), vk::CommandBufferLevel::ePrimary, numFrames};
  cmdBuffers = _device.allocateCommandBuffers(info);
  for(auto i = 0u; i < numFrames; ++i) {
    cmdBuffers[i].begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
    cmdBuffers[i].end();
  }
}

auto Base::resize() -> void {
  device_->vkDevice().waitIdle();
  swapchain_->resize(window_->width(), window_->height(), featureConfig_.vsync);
  //empty cb for syncReverse
  for(auto i = 0u, numFrames = uint32_t(device_->queues().size()); i < numFrames; ++i) {
    cmdBuffers[i].begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
    cmdBuffers[i].end();
  }
}

void Base::onFrame(uint32_t imageIndex, float elapsed) {
  auto &cb = cmdBuffers[frameIndex];

  auto image = swapchain_->image(imageIndex);
  image::setLayout(
    cb, image, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR,
    vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eMemoryRead);
}

void Base::loop(const std::function<void(uint32_t, double)> &updater) {
  onInit();
  auto start = std::chrono::high_resolution_clock::now();
  while(!window_->windowShouldClose()) {
    window_->pollEvents();
    if(window_->resizeWanted()) {
      resize();
      window_->setResizeWanted(false);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<double, std::milli>(end - start).count();
    start = end;

    fpsMeter_.update(elapsed);
    //    syncSemaphore(elapsed, updater);
    syncTimeline(elapsed, updater);
  }

  device_->vkDevice().waitIdle();
  window_->terminate();
}

auto Base::window() -> Window & { return *window_; }
void Base::loop(Updater updater, void *data) {
  loop([&](uint32_t frameIdx, double elapsed) { updater(frameIdx, elapsed, data); });
}
void Base::loop(CallFrameUpdater &updater) {
  loop([&](uint32_t frameIdx, double elapsed) { updater.update(frameIdx, elapsed); });
}
auto Base::device() -> Device & { return *device_; }
auto Base::swapchain() -> Swapchain & { return *swapchain_; }

auto Base::syncSemaphore(
  double elapsed, const std::function<void(uint32_t, double)> &updater) -> void {
  auto &sync = semaphoreSyncs[frameIndex];

  auto resourcesFence = *sync.resourcesFence;
  device_->vkDevice().waitForFences(
    resourcesFence, true, std::numeric_limits<uint64_t>::max());
  device_->vkDevice().resetFences(resourcesFence);

  /**
   * imageIndex is the index of available swapchain image. frameIndex is the ring index of frame.
   * we should depend on frameIndex to ring index our buffers and render into imageIndex swapchain image.
   */
  uint32_t imageIndex = 0;

  try {
    auto result = swapchain_->acquireNextImage(*sync.imageAvailable, imageIndex);
    if(result == vk::Result::eSuboptimalKHR) resize();
  } catch(const vk::OutOfDateKHRError &) { resize(); }

  auto cb = cmdBuffers[frameIndex];

  cb.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  updater(frameIndex, elapsed);
  onFrame(imageIndex, float(elapsed));

  cb.end();

  vk::SubmitInfo submit;
  using vkStage = vk::PipelineStageFlagBits;
  std::vector<vk::PipelineStageFlags> waitStages{vkStage::eColorAttachmentOutput};
  std::vector<vk::Semaphore> waitSemaphores{*sync.imageAvailable};
  std::vector<vk::Semaphore> signalSemaphores{*sync.renderFinished};

  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &cb;
  submit.waitSemaphoreCount = uint32_t(waitSemaphores.size());
  submit.pWaitSemaphores = waitSemaphores.data();
  submit.pWaitDstStageMask = waitStages.data();
  submit.signalSemaphoreCount = uint32_t(signalSemaphores.size());
  submit.pSignalSemaphores = signalSemaphores.data();
  device_->queues()[frameIndex].submit(submit, resourcesFence);

  try {
    auto result = swapchain_->present(frameIndex, imageIndex, *sync.renderFinished);
    if(result == vk::Result::eSuboptimalKHR) resize();
  } catch(const vk::OutOfDateKHRError &) { resize(); }

  frameIndex = (frameIndex + 1) % featureConfig_.numFrames;
}

auto Base::syncTimeline(
  double elapsed, const std::function<void(uint32_t, double)> &updater) -> void {
  auto dev = device_->vkDevice();
  auto &sync = timelineSyncs[frameIndex];

  vk::SemaphoreWaitInfo waitInfo{{}, 1, sync.semaphore.operator->(), &sync.waitValue};

  dev.waitSemaphores(waitInfo, UINT64_MAX);

  /**
     * imageIndex is the index of available swapchain image. frameIndex is the ring index of frame.
     * we should depend on frameIndex to ring index our buffers and render into imageIndex swapchain image.
     */
  uint32_t imageIndex = 0;
  try {
    auto result = swapchain_->acquireNextImage(*sync.wsiImageAvailable, imageIndex);
    if(result == vk::Result::eSuboptimalKHR) resize();
  } catch(const vk::OutOfDateKHRError &) { resize(); }

  auto cb = cmdBuffers[frameIndex];

  cb.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  updater(frameIndex, elapsed);
  onFrame(imageIndex, float(elapsed));

  cb.end();

  const auto lastRenderFinishedValue = sync.waitValue;
  const auto renderFinishedValue = lastRenderFinishedValue + 1;

  sync.waitValue = renderFinishedValue;

  vk::SubmitInfo submit;
  std::vector<vk::PipelineStageFlags> waitStages{
    vk::PipelineStageFlagBits::eAllCommands,
    vk::PipelineStageFlagBits::eColorAttachmentOutput};
  std::vector<vk::Semaphore> waitSemaphores{
    sync.semaphore.get(), sync.wsiImageAvailable.get()};
  std::array<uint64_t, 2> waitValues{lastRenderFinishedValue, 0};

  std::vector<vk::Semaphore> signalSemaphores{
    sync.semaphore.get(), sync.wsiReadyToPresent.get()};
  std::array<uint64_t, 2> signalValues{renderFinishedValue, 0};

  vk::TimelineSemaphoreSubmitInfo timelineInfo;
  timelineInfo.waitSemaphoreValueCount = uint32_t(waitValues.size());
  timelineInfo.pWaitSemaphoreValues = waitValues.data();
  timelineInfo.signalSemaphoreValueCount = uint32_t(signalValues.size());
  timelineInfo.pSignalSemaphoreValues = signalValues.data();

  submit.pNext = &timelineInfo;
  submit.waitSemaphoreCount = uint32_t(waitSemaphores.size());
  submit.pWaitSemaphores = waitSemaphores.data();
  submit.pWaitDstStageMask = waitStages.data();
  submit.signalSemaphoreCount = uint32_t(signalSemaphores.size());
  submit.pSignalSemaphores = signalSemaphores.data();
  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &cb;
  device_->queues()[frameIndex].submit(submit, nullptr);

  try {
    auto result = swapchain_->present(frameIndex, imageIndex, *sync.wsiReadyToPresent);
    if(result == vk::Result::eSuboptimalKHR) resize();
  } catch(const vk::OutOfDateKHRError &) { resize(); }

  frameIndex = (frameIndex + 1) % featureConfig_.numFrames;
}
auto Base::fpsMeter() -> FPSMeter & { return fpsMeter_; }
}