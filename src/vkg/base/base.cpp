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
  createDebutUtils();
  device = std::make_unique<Device>(*instance, window_->vkSurface(), featureConfig);
  swapchain = std::make_unique<Swapchain>(*device, window_->vkSurface(), windowConfig);
  Base::resize();
  createSyncObjects();
  createCommandBuffers();
}

auto Base::featureConfig() const -> FeatureConfig { return featureConfig_; }

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}
auto Base::createDebutUtils() -> void {
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
  auto _device = device->vkDevice();
  auto numFrames = swapchain->imageCount();

  semaphores.resize(numFrames);
  inFlightFrameFences.resize(numFrames);
  vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};
  for(uint32_t i = 0; i < numFrames; i++) {
    semaphores[i].imageAvailable = _device.createSemaphoreUnique({});
    semaphores[i].computeFinished = _device.createSemaphoreUnique({});
    semaphores[i].renderFinished = _device.createSemaphoreUnique({});

    semaphores[i].renderWaits.push_back(*semaphores[i].imageAvailable);
    semaphores[i].renderWaits.push_back(*semaphores[i].computeFinished);
    semaphores[i].renderWaitStages.emplace_back(vk::PipelineStageFlagBits::eAllCommands);
    semaphores[i].renderWaitStages.emplace_back(vk::PipelineStageFlagBits::eAllCommands);

    inFlightFrameFences[i] = _device.createFenceUnique(fenceInfo);
  }
}

auto Base::createCommandBuffers() -> void {
  auto _device = device->vkDevice();
  vk::CommandBufferAllocateInfo info{
    device->graphicsCmdPool(), vk::CommandBufferLevel::ePrimary,
    uint32_t(swapchain->imageCount())};
  graphicsCmdBuffers = _device.allocateCommandBuffers(info);
  info.commandPool = device->computeCmdPool();
  computeCmdBuffers = _device.allocateCommandBuffers(info);
}

auto Base::resize() -> void {
  device->vkDevice().waitIdle();
  swapchain->resize(window_->width(), window_->height(), window_->isVsync());
}

void Base::onFrame(uint32_t imageIndex, float elapsed) {
  auto &graphicsCB = graphicsCmdBuffers[imageIndex];
  auto &computeCB = computeCmdBuffers[imageIndex];

  computeCB.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
  graphicsCB.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  auto image = swapchain->image(imageIndex);
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
  device->computeQueue().submit(submit, {});

  submit.pCommandBuffers = &graphicsCB;
  submit.waitSemaphoreCount = uint32_t(semaphore.renderWaits.size());
  submit.pWaitSemaphores = semaphore.renderWaits.data();
  submit.pWaitDstStageMask = semaphore.renderWaitStages.data();
  submit.signalSemaphoreCount = 1;
  submit.pSignalSemaphores = &(*semaphore.renderFinished);
  device->graphicsQueue().submit(submit, {});
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

    uint32_t imageIndex = 0;
    try {
      auto result =
        swapchain->acquireNextImage(*semaphores[frameIndex].imageAvailable, imageIndex);
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

    auto &semaphore = semaphores[frameIndex];
    try {
      auto result = swapchain->present(imageIndex, *semaphore.renderFinished);
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

    frameIndex = (frameIndex + 1) % swapchain->imageCount();
    device->vkDevice().waitIdle();
  }

  device->vkDevice().waitIdle();
  window_->terminate();
}

auto Base::window() -> Window & { return *window_; }
void Base::loop(Updater updater, void *data) {
  loop([&](double elapsed) { updater(elapsed, data); });
}
void Base::loop(CallFrameUpdater &updater) {
  loop([&](double elapsed) { updater.update(elapsed); });
}

}