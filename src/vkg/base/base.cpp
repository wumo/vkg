#include "base.hpp"
#include <iostream>
#include "vkg/util/syntactic_sugar.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace vkg {

Base::Base(WindowConfig windowConfig, FeatureConfig featureConfig)
  : windowConfig_{windowConfig}, featureConfig_{featureConfig} {
  window_ = std::make_unique<Window>(windowConfig);
  instance = std::make_unique<Instance>(featureConfig);
  window_->createSurface(instance->vkInstance());
  createDebugUtils();
  device_ = std::make_unique<Device>(*instance, window_->vkSurface(), featureConfig);
  swapchain_ = std::make_unique<Swapchain>(*device_, window_->vkSurface(), windowConfig);
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
  auto numFrames = swapchain_->imageCount();

  semaphoreSyncs.resize(numFrames);
  vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};
  for(uint32_t i = 0; i < numFrames; i++) {
    semaphoreSyncs[i].imageAvailable = _device.createSemaphoreUnique({});
    semaphoreSyncs[i].computeFinished = _device.createSemaphoreUnique({});
    semaphoreSyncs[i].readyToCompute = _device.createSemaphoreUnique({});
    semaphoreSyncs[i].renderFinished = _device.createSemaphoreUnique({});

    if(i == 0) { // first frame requires computeFinished
      vk::SubmitInfo submit;
      submit.signalSemaphoreCount = 1;
      submit.pSignalSemaphores = semaphoreSyncs[i].computeFinished.operator->();
      device_->computeQueue().submit(submit, {});
    } else { // following frames require readyToCompute
      vk::SubmitInfo submit;
      submit.signalSemaphoreCount = 1;
      submit.pSignalSemaphores = semaphoreSyncs[i].readyToCompute.operator->();
      device_->computeQueue().submit(submit, {});
    }

    semaphoreSyncs[i].resourcesFence = _device.createFenceUnique(fenceInfo);
    if(i != 0) _device.resetFences(semaphoreSyncs[i].resourcesFence.get());
  }

  timelineSyncs.resize(numFrames);
  for(auto i = 0u; i < numFrames; ++i) {
    vk::SemaphoreTypeCreateInfo timelineCreateInfo{vk::SemaphoreType::eTimeline, 0};
    vk::SemaphoreCreateInfo createInfo{};
    createInfo.pNext = &timelineCreateInfo;
    timelineSyncs[i].semaphore = _device.createSemaphoreUnique(createInfo);
  }
}

auto Base::createCommandBuffers() -> void {
  auto _device = device_->vkDevice();
  vk::CommandBufferAllocateInfo info{
    device_->graphicsCmdPool(), vk::CommandBufferLevel::ePrimary,
    uint32_t(swapchain_->imageCount())};
  graphicsCmdBuffers = _device.allocateCommandBuffers(info);
  info.commandPool = device_->computeCmdPool();
  computeCmdBuffers = _device.allocateCommandBuffers(info);
  for(auto i = 0u; i < uint32_t(swapchain_->imageCount()); ++i) {
    graphicsCmdBuffers[i].begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
    graphicsCmdBuffers[i].end();
    computeCmdBuffers[i].begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
    computeCmdBuffers[i].end();
  }
}

auto Base::resize() -> void {
  device_->vkDevice().waitIdle();
  swapchain_->resize(window_->width(), window_->height(), window_->isVsync());
  //empty cb for syncReverse
  for(auto i = 0u; i < uint32_t(swapchain_->imageCount()); ++i) {
    graphicsCmdBuffers[i].begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
    graphicsCmdBuffers[i].end();
    computeCmdBuffers[i].begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
    computeCmdBuffers[i].end();
  }
}

void Base::onFrame(uint32_t imageIndex, float elapsed) {
  auto &graphicsCB = graphicsCmdBuffers[frameIndex];
  auto &computeCB = computeCmdBuffers[frameIndex];

  auto image = swapchain_->image(imageIndex);
  image::setLayout(
    graphicsCB, image, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR,
    vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eMemoryRead);
}

void Base::loop(const std::function<void(double)> &updater) {
  onInit();
  auto start = std::chrono::high_resolution_clock::now();
  while(!window_->windowShouldClose()) {
    window_->setWindowTitle(toString(
      "FPS: ", fpsMeter.fps(), " Frame Time: ", std::round(fpsMeter.frameTime()), " ms"));
    window_->pollEvents();

    if(window_->resizeWanted()) {
      resize();
      window_->setResizeWanted(false);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<double, std::milli>(end - start).count();
    start = end;

    fpsMeter.update(elapsed);
    syncReverse(elapsed, updater);
    //        syncTimeline(elapsed, updater);
  }

  device_->vkDevice().waitIdle();
  window_->terminate();
}

auto Base::window() -> Window & { return *window_; }
void Base::loop(Updater updater, void *data) {
  loop([&](double elapsed) { updater(elapsed, data); });
}
void Base::loop(CallFrameUpdater &updater) {
  loop([&](double elapsed) { updater.update(elapsed); });
}
auto Base::device() -> Device & { return *device_; }
auto Base::swapchain() -> Swapchain & { return *swapchain_; }

auto Base::syncReverse(double elapsed, const std::function<void(double)> &updater)
  -> void {
  vk::SubmitInfo submit;
  std::vector<vk::PipelineStageFlags> waitStages;
  std::vector<vk::Semaphore> waitSemaphores;
  std::vector<vk::Semaphore> signalSemaphores;
  using vkStage = vk::PipelineStageFlagBits;

  auto &semaphore = semaphoreSyncs[frameIndex];

  auto resourcesFence = *semaphore.resourcesFence;
  device_->vkDevice().waitForFences(
    resourcesFence, true, std::numeric_limits<uint64_t>::max());
  device_->vkDevice().resetFences(resourcesFence);

  /**
   * imageIndex is the index of available swapchain image. frameIndex is the ring index of frame.
   * we should depend on frameIndex to ring index our buffers and render into imageIndex swapchain image.
   */
  uint32_t imageIndex = 0;

  try {
    auto result = swapchain_->acquireNextImage(*semaphore.imageAvailable, imageIndex);
    if(result == vk::Result::eSuboptimalKHR) resize();
  } catch(const vk::OutOfDateKHRError &) { resize(); }

  auto graphicsCB = graphicsCmdBuffers[frameIndex];

  waitStages.insert(
    waitStages.end(), {vkStage::eColorAttachmentOutput, vkStage::eComputeShader});
  waitSemaphores.insert(
    waitSemaphores.end(), {*semaphore.imageAvailable, *semaphore.computeFinished});
  signalSemaphores.insert(
    signalSemaphores.end(), {*semaphore.readyToCompute, *semaphore.renderFinished});

  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &graphicsCB;
  submit.waitSemaphoreCount = uint32_t(waitSemaphores.size());
  submit.pWaitSemaphores = waitSemaphores.data();
  submit.pWaitDstStageMask = waitStages.data();
  submit.signalSemaphoreCount = uint32_t(signalSemaphores.size());
  submit.pSignalSemaphores = signalSemaphores.data();
  device_->graphicsQueue().submit(submit, {});

  try {
    auto result = swapchain_->present(imageIndex, *semaphore.renderFinished);
    if(result == vk::Result::eSuboptimalKHR) resize();
  } catch(const vk::OutOfDateKHRError &) { resize(); }

  frameIndex = (frameIndex + 1) % swapchain_->imageCount();

  graphicsCB = graphicsCmdBuffers[frameIndex];
  auto computeCB = computeCmdBuffers[frameIndex];

  computeCB.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
  graphicsCB.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  updater(elapsed);
  onFrame(imageIndex, float(elapsed));

  computeCB.end();
  graphicsCB.end();

  // Wait for rendering finished. TODO have to submit compute cmd after graphics cmd to resolve buffer sync problem. duno why.
  waitStages.clear();
  waitSemaphores.clear();
  signalSemaphores.clear();
  waitStages.emplace_back(vkStage::eComputeShader);
  waitSemaphores.emplace_back(*semaphoreSyncs[frameIndex].readyToCompute);
  signalSemaphores.emplace_back(*semaphoreSyncs[frameIndex].computeFinished);

  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &computeCB;
  submit.waitSemaphoreCount = uint32_t(waitSemaphores.size());
  submit.pWaitSemaphores = waitSemaphores.data();
  submit.pWaitDstStageMask = waitStages.data();
  submit.signalSemaphoreCount = uint32_t(signalSemaphores.size());
  submit.pSignalSemaphores = signalSemaphores.data();
  device_->computeQueue().submit(submit, {*semaphoreSyncs[frameIndex].resourcesFence});
}

/**
 * TODO this method cannot ensure the memory sync between async compute queue and graphics queue
 */
auto Base::syncTimeline(double elapsed, const std::function<void(double)> &updater)
  -> void {
  auto dev = device_->vkDevice();
  auto &tSemaphore = timelineSyncs[frameIndex];
  auto &semaphore = semaphoreSyncs[frameIndex];

  vk::SemaphoreWaitInfo waitInfo{
    {}, 1, tSemaphore.semaphore.operator->(), &tSemaphore.waitValue};

  dev.waitSemaphores(waitInfo, UINT64_MAX);

  /**
   * imageIndex is the index of available swapchain image. frameIndex is the ring index of frame.
   * we should depend on frameIndex to ring index our buffers and render into imageIndex swapchain image.
   */
  uint32_t imageIndex = 0;
  try {
    auto result = swapchain_->acquireNextImage(*semaphore.imageAvailable, imageIndex);
    if(result == vk::Result::eSuboptimalKHR) resize();
  } catch(const vk::OutOfDateKHRError &) { resize(); }

  auto graphicsCB = graphicsCmdBuffers[frameIndex];
  auto computeCB = computeCmdBuffers[frameIndex];

  computeCB.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
  graphicsCB.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  updater(elapsed);
  onFrame(imageIndex, float(elapsed));

  computeCB.end();
  graphicsCB.end();

  const auto lastRenderFinishedValue = tSemaphore.waitValue;
  const auto computeFinishedValue = lastRenderFinishedValue + 1;
  const auto renderFinishedValue = computeFinishedValue + 1;

  tSemaphore.waitValue = renderFinishedValue;

  vk::SubmitInfo submit;
  vk::TimelineSemaphoreSubmitInfo timelineInfo;
  std::vector<vk::PipelineStageFlags> waitStages;
  std::vector<vk::Semaphore> waitSemaphores;
  std::vector<vk::Semaphore> signalSemaphores;

  //compute

  timelineInfo.waitSemaphoreValueCount = 1;
  timelineInfo.pWaitSemaphoreValues = &lastRenderFinishedValue;
  timelineInfo.signalSemaphoreValueCount = 1;
  timelineInfo.pSignalSemaphoreValues = &computeFinishedValue;

  waitSemaphores = {tSemaphore.semaphore.get()};
  waitStages = {vk::PipelineStageFlagBits::eComputeShader};
  signalSemaphores = {tSemaphore.semaphore.get()};
  submit.pNext = &timelineInfo;
  submit.waitSemaphoreCount = uint32_t(waitSemaphores.size());
  submit.pWaitSemaphores = waitSemaphores.data();
  submit.pWaitDstStageMask = waitStages.data();
  submit.signalSemaphoreCount = uint32_t(signalSemaphores.size());
  submit.pSignalSemaphores = signalSemaphores.data();
  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &computeCB;
  device_->computeQueue().submit(submit, nullptr);

  // graphics

  waitSemaphores = {tSemaphore.semaphore.get(), semaphore.imageAvailable.get()};
  waitStages = {
    vk::PipelineStageFlagBits::eAllGraphics,
    vk::PipelineStageFlagBits::eColorAttachmentOutput};
  std::array<uint64_t, 2> waitValues{computeFinishedValue, 0};
  signalSemaphores = {tSemaphore.semaphore.get(), semaphore.renderFinished.get()};
  std::array<uint64_t, 2> signalValues{renderFinishedValue, 0};

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
  submit.pCommandBuffers = &graphicsCB;
  device_->graphicsQueue().submit(submit, nullptr);

  try {
    auto result = swapchain_->present(imageIndex, *semaphore.renderFinished);
    if(result == vk::Result::eSuboptimalKHR) resize();
  } catch(const vk::OutOfDateKHRError &) { resize(); }

  frameIndex = (frameIndex + 1) % swapchain_->imageCount();
  //  dev.waitIdle();
}
}