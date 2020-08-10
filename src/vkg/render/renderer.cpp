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

  frameGraph->addPass("Renderer", {}, *this);

  frameGraph->build();
}

auto Renderer::setup(PassBuilder &builder, const RendererPassIn &inputs)
  -> RendererPassOut {
  passOut.swapchainExtent = builder.create<vk::Extent2D>("swapchainExtent");
  passOut.swapchainFormat = builder.create<vk::Format>("swapchainFormat");
  passOut.swapchainVersion = builder.create<uint64_t>("swapchainVersion");

  for(auto &[name, scene]: scenes)
    frameGraph->addPass(
      name, {passOut.swapchainExtent, passOut.swapchainFormat, passOut.swapchainVersion},
      *scene);

  return passOut;
}
void Renderer::compile(Resources &resources) {
  resources.set(passOut.swapchainExtent, swapchain->imageExtent());
  resources.set(passOut.swapchainFormat, swapchain->format());
  resources.set(passOut.swapchainVersion, swapchain->version());
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