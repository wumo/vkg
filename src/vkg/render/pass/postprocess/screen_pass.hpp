#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "common/quad_vert.hpp"
#include <variant>

namespace vkg {
struct ScreenPassIn {
  FrameGraphResource<Texture *> img;
};
struct ScreenPassOut {
  FrameGraphResource<Texture *> img;
};
template<typename PushConstant = std::monostate>
class ScreenPass: public Pass<ScreenPassIn, ScreenPassOut> {
public:
  explicit ScreenPass(const std::span<const uint32_t> &opcodes): opcodes(opcodes) {}
  void setup(PassBuilder &builder) override {
    builder.read(passIn.img);
    passOut = {
      .img = builder.create<Texture *>("img"),
    };
  }
  void compile(RenderContext &ctx, Resources &resources) override {
    auto *img = resources.get(passIn.img);
    if(!init) {
      init = true;

      setDef.init(ctx.device);
      pipeDef.set(setDef);
      pipeDef.init(ctx.device);

      {
        RenderPassMaker maker;
        auto backImg = maker.attachment(img->format())
                         .samples(vk::SampleCountFlagBits::e1)
                         .loadOp(vk::AttachmentLoadOp::eClear)
                         .storeOp(vk::AttachmentStoreOp::eStore)
                         .stencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                         .stencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                         .initialLayout(vk::ImageLayout::eUndefined)
                         .finalLayout(vk::ImageLayout::eColorAttachmentOptimal)
                         .index();
        maker.subpass(vk::PipelineBindPoint::eGraphics).color(backImg).index();

        maker.dependency(VK_SUBPASS_EXTERNAL, 0)
          .srcStage(vk::PipelineStageFlagBits::eBottomOfPipe)
          .dstStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
          .srcAccess(vk::AccessFlagBits::eMemoryRead)
          .dstAccess(vk::AccessFlagBits::eColorAttachmentWrite)
          .flags(vk::DependencyFlagBits::eByRegion);
        maker.dependency(0, VK_SUBPASS_EXTERNAL)
          .srcStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
          .dstStage(vk::PipelineStageFlagBits::eBottomOfPipe)
          .srcAccess(vk::AccessFlagBits::eColorAttachmentWrite)
          .dstAccess(vk::AccessFlagBits::eMemoryRead)
          .flags(vk::DependencyFlagBits::eByRegion);

        renderPass = maker.createUnique(ctx.device);
      }

      {
        GraphicsPipelineMaker maker(ctx.device);

        maker.layout(pipeDef.layout())
          .renderPass(*renderPass)
          .subpass(0)
          .inputAssembly(vk::PrimitiveTopology::eTriangleList)
          .polygonMode(vk::PolygonMode::eFill)
          .cullMode(vk::CullModeFlagBits::eNone)
          .frontFace(vk::FrontFace::eClockwise)
          .depthTestEnable(false)
          .viewport({})
          .scissor({})
          .dynamicState(vk::DynamicState::eViewport)
          .dynamicState(vk::DynamicState::eScissor);

        maker.blendColorAttachment(false);

        maker
          .shader(
            vk::ShaderStageFlagBits::eVertex, Shader{shader::common::quad_vert_span})
          .shader(vk::ShaderStageFlagBits::eFragment, Shader{opcodes});
        pipe = maker.createUnique();
      }

      descriptorPool = DescriptorPoolMaker()
                         .pipelineLayout(pipeDef, ctx.numFrames)
                         .createUnique(ctx.device);

      frames.resize(ctx.numFrames);

      for(auto &frame: frames) {

        frame.set = setDef.createSet(*descriptorPool);
      }
    }
    auto &frame = frames[ctx.frameIndex];

    if(img != frame.img) {
      frame.img = img;
      auto w = img->extent().width;
      auto h = img->extent().height;
      using vkUsage = vk::ImageUsageFlagBits;
      frame.toImg = image::make2DTex(
        "toImg", ctx.device, w, h,
        vkUsage::eSampled | vkUsage::eStorage | vkUsage::eTransferSrc |
          vkUsage ::eTransferDst | vkUsage ::eColorAttachment,
        img->format());
      std::vector<vk::ImageView> attachments = {frame.toImg->imageView()};

      vk::FramebufferCreateInfo info{
        {}, *renderPass, uint32_t(attachments.size()), attachments.data(), w, h, 1};

      frame.framebuffer = ctx.device.vkDevice().createFramebufferUnique(info);
    }

    setDef.image(*img);
    setDef.update(frame.set);

    resources.set(passOut.img, frame.toImg.get());
  }
  void execute(RenderContext &ctx, Resources &resources) override {
    auto &frame = frames[ctx.frameIndex];
    auto *img = resources.get(passIn.img);

    auto cb = ctx.cb;
    ctx.device.begin(cb, name);

    image::transitTo(
      cb, *img, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderRead,
      vk::PipelineStageFlagBits::eFragmentShader);
    image::transitTo(
      cb, *frame.toImg, vk::ImageLayout::eColorAttachmentOptimal,
      vk::AccessFlagBits::eColorAttachmentWrite,
      vk::PipelineStageFlagBits::eColorAttachmentOutput);

    std::array<vk::ClearValue, 1> clearValues{
      vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 0.0f}},
    };

    vk::RenderPassBeginInfo renderPassBeginInfo{
      *renderPass, *frame.framebuffer,
      vk::Rect2D{{0, 0}, {frame.toImg->extent().width, frame.toImg->extent().height}},
      uint32_t(clearValues.size()), clearValues.data()};

    cb.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    vk::Viewport viewport{
      0,    0,   float(frame.toImg->extent().width), float(frame.toImg->extent().height),
      0.0f, 1.0f};
    cb.setViewport(0, viewport);
    vk::Rect2D scissor{
      {0, 0}, {frame.toImg->extent().width, frame.toImg->extent().height}};
    cb.setScissor(0, scissor);

    cb.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, pipeDef.layout(), pipeDef.set.set(), frame.set,
      nullptr);
    if(!std::is_empty_v<PushConstant>) {
      updatePushConstant();
      cb.pushConstants<PushConstant>(
        pipeDef.layout(), vk::ShaderStageFlagBits::eFragment, 0, pushConstant);
    }
    cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipe);
    cb.draw(3, 1, 0, 0);

    cb.endRenderPass();
    ctx.device.end(cb);
  }

protected:
  virtual void updatePushConstant(){};
  PushConstant pushConstant;

private:
  struct ScreenSetDef: DescriptorSetDef {
    __sampler2D__(image, vkStage::eFragment);
  } setDef;

  struct ScreenPipeDef: PipelineLayoutDef {
    __push_constant__(constant, vkStage::eFragment, PushConstant);
    __set__(set, ScreenSetDef);
  } pipeDef;

  vk::UniqueRenderPass renderPass;
  vk::UniquePipeline pipe;

  vk::UniqueDescriptorPool descriptorPool;

  struct FrameResource {
    vk::DescriptorSet set;
    std::unique_ptr<Texture> toImg;
    Texture *img{};
    vk::UniqueFramebuffer framebuffer;
  };
  std::vector<FrameResource> frames;
  bool init{false};

  std::span<const uint32_t> opcodes;
};
}
