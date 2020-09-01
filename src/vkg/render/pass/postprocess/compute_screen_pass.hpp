#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include <variant>

namespace vkg {
struct CompScreenPassIn {
  FrameGraphResource<Texture *> img;
};
struct CompScreenPassOut {
  FrameGraphResource<Texture *> img;
};
template<typename PushConstant = std::monostate>
class CompScreenPass: public Pass<CompScreenPassIn, CompScreenPassOut> {
public:
  explicit CompScreenPass(const std::span<const uint32_t> &opcodes): opcodes(opcodes) {}
  void setup(PassBuilder &builder) override {
    passOut = {
      .img = builder.write(passIn.img),
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

    auto *img = resources.get(passIn.img);
    setDef.img(img->imageView());
    setDef.update(frame.set);

    resources.set(passOut.img, img);
  }
  void execute(RenderContext &ctx, Resources &resources) override {
    auto &frame = frames[ctx.frameIndex];
    auto *img = resources.get(passIn.img);

    auto cb = ctx.cb;
    ctx.device.begin(cb, name);

    image::transitTo(
      cb, *img, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
      vk::PipelineStageFlagBits::eComputeShader);

    cb.bindPipeline(vk::PipelineBindPoint::eCompute, *pipe);
    cb.bindDescriptorSets(
      vk::PipelineBindPoint::eCompute, pipeDef.layout(), pipeDef.set.set(), frame.set,
      nullptr);
    if(!std::is_empty_v<PushConstant>) {
      updatePushConstant();
      cb.pushConstants<PushConstant>(
        pipeDef.layout(), vk::ShaderStageFlagBits::eCompute, 0, pushConstant);
    }
    auto extent = img->extent();
    auto dx = uint32_t(std::ceil(extent.width / double(local_size_x)));
    auto dy = uint32_t(std::ceil(extent.width / double(local_size_y)));
    cb.dispatch(dx, dy, 1);

    ctx.device.end(cb);
  }

protected:
  virtual void updatePushConstant(){};
  PushConstant pushConstant;

private:
  struct CompScreenSetDef: DescriptorSetDef {
    __image2D__(img, vkStage::eCompute);
  } setDef;

  struct CompScreenPipeDef: PipelineLayoutDef {
    __push_constant__(constant, vkStage::eCompute, PushConstant);
    __set__(set, CompScreenSetDef);
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
