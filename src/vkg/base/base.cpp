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
  Base::resize();
  createSyncObjects();
  createCommandBuffers();
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

  semaphores.resize(numFrames);
  renderFences.resize(numFrames);
  vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};
  for(uint32_t i = 0; i < numFrames; i++) {
    semaphores[i].imageAvailable = _device.createSemaphoreUnique({});
    semaphores[i].computeFinished = _device.createSemaphoreUnique({});
    semaphores[i].renderFinished = _device.createSemaphoreUnique({});

    semaphores[i].renderWaits.push_back(*semaphores[i].imageAvailable);
    semaphores[i].renderWaits.push_back(*semaphores[i].computeFinished);
    semaphores[i].renderWaitStages.emplace_back(vk::PipelineStageFlagBits::eAllCommands);
    semaphores[i].renderWaitStages.emplace_back(vk::PipelineStageFlagBits::eAllCommands);

    renderFences[i] = _device.createFenceUnique(fenceInfo);
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
}

auto Base::resize() -> void {
  device_->vkDevice().waitIdle();
  swapchain_->resize(window_->width(), window_->height(), window_->isVsync());
}

void Base::onFrame(uint32_t imageIndex, float elapsed) {
  auto &graphicsCB = graphicsCmdBuffers[frameIndex];
  auto &computeCB = computeCmdBuffers[frameIndex];

  computeCB.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
  graphicsCB.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  auto image = swapchain_->image(imageIndex);
  image::setLayout(
    graphicsCB, image, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR,
    vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eMemoryRead);

  computeCB.end();
  graphicsCB.end();

  auto &semaphore = semaphores[frameIndex];

  vk::SubmitInfo submit;
  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &computeCB;
  submit.signalSemaphoreCount = 1;
  submit.pSignalSemaphores = &(*semaphore.computeFinished);
  device_->computeQueue().submit(submit, {});

  submit.pCommandBuffers = &graphicsCB;
  submit.waitSemaphoreCount = uint32_t(semaphore.renderWaits.size());
  submit.pWaitSemaphores = semaphore.renderWaits.data();
  submit.pWaitDstStageMask = semaphore.renderWaitStages.data();
  submit.signalSemaphoreCount = 1;
  submit.pSignalSemaphores = &(*semaphore.renderFinished);
  device_->graphicsQueue().submit(submit, *renderFences[frameIndex]);
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

    auto renderFence = *renderFences[frameIndex];
    device_->vkDevice().waitForFences(
      renderFence, true, std::numeric_limits<uint64_t>::max());
    device_->vkDevice().resetFences(renderFence);

    /**
     * imageIndex is the index of available swapchain image. frameIndex is the ring index of frame.
     * we should depend on frameIndex to ring index our buffers and render into imageIndex swapchain image.
     */
    uint32_t imageIndex = 0;
    auto &semaphore = semaphores[frameIndex];
    try {
      auto result = swapchain_->acquireNextImage(*semaphore.imageAvailable, imageIndex);
      if(result == vk::Result::eSuboptimalKHR) {
        window_->setResizeWanted(true);
        resize();
        window_->setResizeWanted(false);
      }
    } catch(const vk::OutOfDateKHRError &) {
      window_->setResizeWanted(true);
      resize();
      window_->setResizeWanted(false);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<double, std::milli>(end - start).count();
    start = end;

    fpsMeter.update(elapsed);

    updater(elapsed);
    onFrame(imageIndex, float(elapsed));

    try {
      auto result = swapchain_->present(imageIndex, *semaphore.renderFinished);
      if(result == vk::Result::eSuboptimalKHR) {
        window_->setResizeWanted(true);
        resize();
        window_->setResizeWanted(false);
      }
    } catch(const vk::OutOfDateKHRError &) {
      window_->setResizeWanted(true);
      resize();
      window_->setResizeWanted(false);
    }

    frameIndex = (frameIndex + 1) % swapchain_->imageCount();
    device_->vkDevice().waitIdle();
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

}