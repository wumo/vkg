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
  void setup(PassBuilder &builder) override {
    passOut = {
      .swapchainExtent = builder.create<vk::Extent2D>("swapchainExtent"),
      .swapchainFormat = builder.create<vk::Format>("swapchainFormat"),
      .swapchainVersion = builder.create<uint64_t>("swapchainVersion"),
    };
  }
  void compile(RenderContext &ctx, Resources &resources) override {
    resources.set(passOut.swapchainExtent, renderer.swapchain().imageExtent());
    resources.set(passOut.swapchainFormat, renderer.swapchain().format());
    resources.set(passOut.swapchainVersion, renderer.swapchain().version());
  }

private:
  Renderer &renderer;
};

struct RendererPresentPassIn {
  std::vector<FrameGraphResource<Texture *>> backImgs;
  std::vector<FrameGraphResource<vk::Rect2D>> renderAreas;
};
struct RendererPresentPassOut {};

class RendererPresentPass: public Pass<RendererPresentPassIn, RendererPresentPassOut> {
public:
  explicit RendererPresentPass(Renderer &renderer): renderer(renderer) {}
  void setup(PassBuilder &builder) override {
    auto n = passIn.backImgs.size();
    for(int i = 0; i < n; ++i) {
      builder.read(passIn.backImgs[i]);
      builder.read(passIn.renderAreas[i]);
    }
  }
  void execute(RenderContext &ctx, Resources &resources) override {
    auto cb = ctx.cb;

    auto present = renderer.swapchain().image(ctx.swapchainIndex);
    image::setLayout(
      cb, present, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, {},
      vk::AccessFlagBits::eTransferWrite);

    auto n = passIn.backImgs.size();
    for(int i = 0; i < n; ++i) {
      auto backImg = resources.get(passIn.backImgs[i]);
      auto renderArea = resources.get(passIn.renderAreas[i]);
      ctx.device.begin(cb, toString("barrier backImg frame ", ctx.frameIndex));
      image::transitTo(
        cb, *backImg, vk::ImageLayout::eTransferSrcOptimal,
        vk::AccessFlagBits::eTransferRead, vk::PipelineStageFlagBits::eTransfer);
      ctx.device.end(cb);
      ctx.device.begin(cb, toString("copy backImg frame ", ctx.frameIndex));
      image::blit(
        cb, present,
        {vk::Offset3D{renderArea.offset, 0},
         vk::Offset3D{
           int32_t(renderArea.extent.width), int32_t(renderArea.extent.height), 1}},
        *backImg,
        {vk::Offset3D{},
         vk::Offset3D{
           int32_t(renderArea.extent.width), int32_t(renderArea.extent.height), 1}});
      ctx.device.end(cb);
    }
    ctx.device.begin(cb, toString("barrier present frame ", ctx.frameIndex));
    image::setLayout(
      cb, present, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR,
      vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eMemoryRead);
    ctx.device.end(cb);
  }

private:
  Renderer &renderer;
};

Renderer::Renderer(WindowConfig windowConfig, FeatureConfig featureConfig)
  : Base(std::move(windowConfig), featureConfig) {}

auto Renderer::addScene(SceneConfig sceneConfig, const std::string &name) -> Scene & {
  scenes[name] = std::make_unique<Scene>(*this, sceneConfig, name);
  return *scenes[name];
}

void Renderer::onInit() {
  frameGraph = std::make_unique<FrameGraph>(*device_);

  auto &pass = frameGraph->newPass<RendererSetupPass>("RendererSetup", {}, *this);

  RendererPresentPassIn presentPassIn;
  for(auto &[name, scene]: scenes) {
    auto &scenePass = frameGraph->addPass(
      name,
      ScenePassIn{
        pass.out().swapchainExtent, pass.out().swapchainFormat,
        pass.out().swapchainVersion},
      *scene);
    presentPassIn.backImgs.push_back(scenePass.out().backImg);
    presentPassIn.renderAreas.push_back(scenePass.out().renderArea);
  }

  frameGraph->newPass<RendererPresentPass>("RendererPresent", presentPassIn, *this);

  frameGraph->build();
}

void Renderer::onFrame(uint32_t imageIndex, float elapsed) {
  println("begin frame ");
  RenderContext ctx{
    *device_, imageIndex, frameIndex, uint32_t(device_->queues().size()),
    cmdBuffers[frameIndex]};
  frameGraph->onFrame(ctx);
  println("end frmae");
}
}