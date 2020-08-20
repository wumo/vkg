#include "tonemap_pass.hpp"
#include "postprocess/tonemap_comp.hpp"

namespace vkg {
void ToneMapPass::setup(PassBuilder &builder) {
  passOut = {
    .backImg = builder.write(passIn.backImg),
  };
}
void ToneMapPass::compile(RenderContext &ctx, Resources &resources) {
  if(!init) {
    init = true;

    setDef.init(ctx.device);
    pipeDef.set(setDef);
    pipeDef.init(ctx.device);

    pipe = ComputePipelineMaker(ctx.device)
             .layout(pipeDef.layout())
             .shader(Shader{shader::postprocess::tonemap_comp_span, 1, 1, 1})
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
void ToneMapPass::execute(RenderContext &ctx, Resources &resources) {
  auto &frame = frames[ctx.frameIndex];
  auto *backImg = resources.get(passIn.backImg);

  auto cb = ctx.graphics;
  ctx.device.begin(cb, "compute tonemap");

  image::transitTo(
    cb, *backImg, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
    vk::PipelineStageFlagBits::eComputeShader);

  cb.bindPipeline(vk::PipelineBindPoint::eCompute, *pipe);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eCompute, pipeDef.layout(), pipeDef.set.set(), frame.set,
    nullptr);
  cb.pushConstants<PushConstant>(
    pipeDef.layout(), vk::ShaderStageFlagBits::eCompute, 0, pushConstant);
  auto extent = backImg->extent();
  cb.dispatch(extent.width, extent.height, 1);

  ctx.device.end(cb);
}
}