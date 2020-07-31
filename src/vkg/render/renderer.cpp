#include "renderer.hpp"

#include <utility>

namespace vkg {
Renderer::Renderer(WindowConfig windowConfig, FeatureConfig featureConfig)
  : Base(std::move(windowConfig), featureConfig) {}

auto Renderer::addScene(SceneConfig sceneConfig, const std::string &name) -> Scene & {
  scenes[name] = std::make_unique<Scene>(*this, sceneConfig, name);
  return *scenes[name];
}

void Renderer::onInit() {
  frameGraph = std::make_unique<FrameGraph>(*device);

  frameGraph->addPass(
    "Renderer",
    [&](PassBuilder &builder) {
      extent = builder.create("SwapchainExtent", ResourceType::eValue);
    },
    [&](Resources &resources) { resources.set(extent, swapchain->imageExtent()); });
  for(auto &[_, scene]: scenes)
    scene->addPass(*frameGraph, {extent});

  frameGraph->build();
}

void Renderer::onFrame(uint32_t imageIndex, float elapsed) {
  auto &graphicsCB = graphicsCmdBuffers[imageIndex];
  auto &computeCB = computeCmdBuffers[imageIndex];

  computeCB.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
  graphicsCB.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  frameGraph->onFrame(graphicsCB, computeCB);

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

}