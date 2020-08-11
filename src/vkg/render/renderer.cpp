#include "renderer.hpp"

#include <utility>

namespace vkg {
struct RendererSetupPassIn {};
struct RendererSetupPassOut {
  FrameGraphResource<vk::Extent2D> swapchainExtent;
  FrameGraphResource<vk::Format> swapchainFormat;
  FrameGraphResource<uint64_t> swapchainVersion;
};
class RendererSetupPass: public Pass<RendererSetupPassIn, RendererSetupPassOut> {
public:
  explicit RendererSetupPass(Renderer &renderer): renderer(renderer) {}
  auto setup(PassBuilder &builder, const RendererSetupPassIn &inputs)
    -> RendererSetupPassOut override {
    passOut.swapchainExtent = builder.create<vk::Extent2D>("swapchainExtent");
    passOut.swapchainFormat = builder.create<vk::Format>("swapchainFormat");
    passOut.swapchainVersion = builder.create<uint64_t>("swapchainVersion");
    return passOut;
  }
  void compile(Resources &resources) override {
    resources.set(passOut.swapchainExtent, renderer.swapchain().imageExtent());
    resources.set(passOut.swapchainFormat, renderer.swapchain().format());
    resources.set(passOut.swapchainVersion, renderer.swapchain().version());
  }

private:
  Renderer &renderer;
  RendererSetupPassOut passOut;
};

struct RendererPresentPassIn {
  std::vector<FrameGraphResource<Texture *>> backImgs;
  std::vector<FrameGraphResource<vk::Rect2D>> renderAreas;
};
struct RendererPresentPassOut {};

class RendererPresentPass: public Pass<RendererPresentPassIn, RendererPresentPassOut> {
public:
  explicit RendererPresentPass(Renderer &renderer): renderer(renderer) {}
  auto setup(PassBuilder &builder, const RendererPresentPassIn &inputs)
    -> RendererPresentPassOut override {
    passIn = inputs;
    auto n = passIn.backImgs.size();
    for(int i = 0; i < n; ++i) {
      builder.read(passIn.backImgs[i]);
      builder.read(passIn.renderAreas[i]);
    }
    return {};
  }
  void execute(RenderContext &ctx, Resources &resources) override {
    auto cb = ctx.graphics;
    auto present = renderer.swapchain().image(ctx.swapchainIndex);
    image::setLayout(
      cb, present, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, {},
      vk::AccessFlagBits::eTransferWrite);

    auto n = passIn.backImgs.size();
    for(int i = 0; i < n; ++i) {
      auto backImg = resources.get(passIn.backImgs[i]);
      auto renderArea = resources.get(passIn.renderAreas[i]);
      image::transitTo(
        cb, *backImg, vk::ImageLayout::eTransferSrcOptimal,
        vk::AccessFlagBits::eTransferRead, vk::PipelineStageFlagBits::eTransfer);

      image::copy(cb, present, *backImg);
    }
    image::setLayout(
      cb, present, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR,
      vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eMemoryRead);
  }

private:
  Renderer &renderer;
  RendererPresentPassIn passIn;
};

Renderer::Renderer(WindowConfig windowConfig, FeatureConfig featureConfig)
  : Base(std::move(windowConfig), featureConfig) {}

auto Renderer::addScene(SceneConfig sceneConfig, const std::string &name) -> Scene & {
  scenes[name] = std::make_unique<Scene>(*this, sceneConfig, name);
  return *scenes[name];
}

void Renderer::onInit() {
  frameGraph = std::make_unique<FrameGraph>(*device_);

  auto passOut =
    frameGraph->newPass<RendererSetupPass>("RendererSetup", RendererSetupPassIn{}, *this);

  RendererPresentPassIn presentPassIn;
  for(auto &[name, scene]: scenes) {
    auto scenePassOut = frameGraph->addPass(
      name, {passOut.swapchainExtent, passOut.swapchainFormat, passOut.swapchainVersion},
      *scene);
    presentPassIn.backImgs.push_back(scenePassOut.backImg);
    presentPassIn.renderAreas.push_back(scenePassOut.renderArea);
  }

  frameGraph->newPass<RendererPresentPass>("RendererPresent", presentPassIn, *this);

  frameGraph->build();
}

void Renderer::onFrame(uint32_t imageIndex, float elapsed) {
  auto &graphicsCB = graphicsCmdBuffers[imageIndex];
  auto &computeCB = computeCmdBuffers[imageIndex];

  computeCB.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
  graphicsCB.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  frameGraph->onFrame(imageIndex, graphicsCB, computeCB);

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
  device_->graphicsQueue().submit(submit, {});
}

}