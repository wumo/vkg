#include "compute_cull_drawcmd.hpp"
#include "common/cull_draw_group_comp.hpp"
#include <utility>

namespace vkg {

auto ComputeCullDrawCMD::setup(
  PassBuilder &builder, const ComputeCullDrawCMDPassIn &inputs)
  -> ComputeCullDrawCMDPassOut {
  passIn = inputs;

  builder.read(passIn.frustum);
  builder.read(passIn.meshInstances);
  builder.read(passIn.meshInstancesCount);
  builder.read(passIn.primitives);
  builder.read(passIn.matrices);
  builder.read(passIn.drawGroupCount);
  passOut.drawCMDBuffer = builder.write(passIn.drawCMDBuffer);
  passOut.drawCMDCountBuffer = builder.write(passIn.drawCMDCountBuffer);

  setDef.init(builder.device());
  pipeDef.transf(setDef);
  pipeDef.init(builder.device());
  descriptorPool = DescriptorPoolMaker().setLayout(setDef).createUnique(builder.device());

  pipe = ComputePipelineMaker(builder.device())
           .layout(pipeDef.layout())
           .shader(Shader{shader::common::cull_draw_group_comp_span, local_size, 1, 1})
           .createUnique();

  return passOut;
}
void ComputeCullDrawCMD::compile(Resources &resources) {
  if(!set) {
    auto drawGroupCount = resources.get(passIn.drawGroupCount);
    drawCMDOffset = buffer::devStorageBuffer(
      resources.device, sizeof(uint32_t) * drawGroupCount.size(), "drawCMDOffset");
    set = setDef.createSet(*descriptorPool);
    setDef.frustum(resources.get(passIn.frustum));
    setDef.meshInstances(resources.get(passIn.meshInstances));
    setDef.primitives(resources.get(passIn.primitives));
    setDef.matrices(resources.get(passIn.matrices));
    setDef.drawCMDOffset(drawCMDOffset->buffer());
    setDef.drawCMD(resources.get(passIn.drawCMDBuffer));
    setDef.drawCMDCount(resources.get(passIn.drawCMDCountBuffer));
    setDef.update(set);
  }
}
void ComputeCullDrawCMD::execute(RenderContext &ctx, Resources &resources) {
  auto total = resources.get(passIn.meshInstancesCount);
  if(total == 0) return;

  auto cb = ctx.compute;

  auto drawGroupCount = resources.get(passIn.drawGroupCount);
  std::vector<uint32_t> offsets(drawGroupCount.size());
  uint32_t offset = 0;
  for(int i = 0; i < drawGroupCount.size(); ++i) {
    offsets[i] = offset;
    offset += drawGroupCount[i];
  }
  cb.updateBuffer(
    drawCMDOffset->buffer(), 0, sizeof(uint32_t) * offsets.size(), offsets.data());

  auto maxCG = ctx.device.limits().maxComputeWorkGroupCount;
  auto totalGroup = uint32_t(std::ceil(total / double(local_size)));
  auto dx = std::min(totalGroup, maxCG[0]);
  totalGroup = uint32_t(totalGroup / double(dx));
  auto dy = std::min(std::max(totalGroup, 1u), maxCG[1]);
  totalGroup = uint32_t(totalGroup / double(dy));
  auto dz = std::min(std::max(totalGroup, 1u), maxCG[1]);

  ctx.device.begin(cb, "compute cull drawGroup");
  cb.bindPipeline(vk::PipelineBindPoint::eCompute, *pipe);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eCompute, pipeDef.layout(), pipeDef.transf.set(), set,
    nullptr);
  cb.pushConstants<uint32_t>(
    pipeDef.layout(), vk::ShaderStageFlagBits::eCompute, 0, total);
  cb.dispatch(dx, dy, dz);

  vk::MemoryBarrier barrier{
    vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead};
  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eAllCommands,
    {}, barrier, nullptr, nullptr);
  ctx.device.end(cb);
}

}