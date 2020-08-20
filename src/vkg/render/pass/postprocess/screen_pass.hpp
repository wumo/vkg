#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"

namespace vkg {
struct ScreenPassIn {
  FrameGraphResource<Texture *> backImg;
};
struct ScreenPassOut {
  FrameGraphResource<Texture *> backImg;
};
struct EmptyPushConstant {};
template<typename PushConstant = EmptyPushConstant>
class ScreenPass: public Pass<ScreenPassIn, ScreenPassOut> {
public:
  explicit ScreenPass(const std::span<const uint32_t> &opcodes): opcodes(opcodes) {}
  void setup(PassBuilder &builder) override {
    passOut = {
      .backImg = builder.write(passIn.backImg),
    };
  }
  void compile(RenderContext &ctx, Resources &resources) override {
    if(!init) {
      init = true;

      setDef.init(ctx.device);
      pipeDef.set(setDef);
      pipeDef.init(ctx.device);

      pipe = ComputePipelineMaker(ctx.device)
               .layout(pipeDef.layout())
               .shader(Shader{opcodes, local_size_x, local_size_y, 1})
               .createUnique();

      descriptorPool = DescriptorPoolMaker()
                         .pipelineLayout(pipeDef, ctx.numFrames)
                         .createUnique(ctx.device);

      frames.resize(ctx.numFrames);
      for(auto &frame: frames)
        frame.set = setDef.createSet(*descriptorPool);
    }
    auto &frame = frames[ctx.frameIndex];

    auto *backImg = resources.get(passIn.backImg);
    setDef.image(backImg->imageView());
    setDef.update(frame.set);

    resources.set(passOut.backImg, backImg);
  }
  void execute(RenderContext &ctx, Resources &resources) override {
    auto &frame = frames[ctx.frameIndex];
    auto *backImg = resources.get(passIn.backImg);

    auto cb = ctx.graphics;
    ctx.device.begin(cb, name);

    image::transitTo(
      cb, *backImg, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
      vk::PipelineStageFlagBits::eComputeShader);

    cb.bindPipeline(vk::PipelineBindPoint::eCompute, *pipe);
    cb.bindDescriptorSets(
      vk::PipelineBindPoint::eCompute, pipeDef.layout(), pipeDef.set.set(), frame.set,
      nullptr);
    if(!std::is_empty_v<PushConstant>)
      cb.pushConstants<PushConstant>(
        pipeDef.layout(), vk::ShaderStageFlagBits::eCompute, 0, pushConstant);
    auto extent = backImg->extent();
    auto maxCG = ctx.device.limits().maxComputeWorkGroupCount;
    auto dx = uint32_t(std::ceil(extent.width / double(local_size_x)));
    auto dy = uint32_t(std::ceil(extent.width / double(local_size_y)));
    cb.dispatch(dx, dy, 1);

    ctx.device.end(cb);
  }

protected:
  virtual void updatePushConstant(){};
  PushConstant pushConstant;

private:
  struct ToneMapSetDef: DescriptorSetDef {
    __image2D__(image, vkStage::eCompute);
  } setDef;

  struct ToneMapPipeDef: PipelineLayoutDef {
    __push_constant__(constant, vkStage::eCompute, PushConstant);
    __set__(set, ToneMapSetDef);
  } pipeDef;

  vk::UniquePipeline pipe;
  const uint32_t local_size_x = 8;
  const uint32_t local_size_y = 8;

  vk::UniqueDescriptorPool descriptorPool;

  struct FrameResource {
    vk::DescriptorSet set;
  };
  std::vector<FrameResource> frames;
  bool init{false};

  std::span<const uint32_t> opcodes;
};
}
