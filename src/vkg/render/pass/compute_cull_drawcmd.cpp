#include "compute_cull_drawcmd.hpp"
#include "common/cull_draw_group_comp.hpp"
#include <utility>

namespace vkg {

ComputeCullDrawCMD::ComputeCullDrawCMD(ComputeCullDrawCMD::PassIn passIn)
  : passIn(std::move(passIn)) {}

void ComputeCullDrawCMD::setup(PassBuilder &builder) {
  setDef.init(builder.device());
  pipeDef.transf(setDef);
  pipeDef.init(builder.device());
  descriptorPool = DescriptorPoolMaker().setLayout(setDef).createUnique(builder.device());

  pipe = ComputePipelineMaker(builder.device())
           .layout(pipeDef.layout())
           .shader(Shader{shader::common::cull_draw_group_comp_span, local_size, 1, 1})
           .createUnique();

  builder.read(passIn.frustum);
  builder.read(passIn.meshInstances);
  builder.read(passIn.meshInstancesCount);
  builder.read(passIn.primitives);
  builder.read(passIn.matrices);
  builder.write(passIn.drawCMDBuffer);
  builder.write(passIn.drawCMDCountBuffer);
  for(auto &drawGroupCount: passIn.drawGroupCount)
    builder.read(drawGroupCount);
}
void ComputeCullDrawCMD::compile(Resources &resources) {
  if(!set) {
    drawCMDOffset = buffer::hostStorageBuffer(
      resources.device, sizeof(uint32_t) * passIn.drawGroupCount.size(), "drawCMDOffset");
    set = setDef.createSet(*descriptorPool);
    setDef.frustum(resources.get<vk::Buffer>(passIn.frustum));
    setDef.meshInstances(resources.get<vk::Buffer>(passIn.meshInstances));
    setDef.primitives(resources.get<vk::Buffer>(passIn.primitives));
    setDef.matrices(resources.get<vk::Buffer>(passIn.matrices));
    setDef.drawCMDOffset(drawCMDOffset->buffer());
    setDef.drawCMD(resources.get<vk::Buffer>(passIn.drawCMDBuffer));
    setDef.drawCMDCount(resources.get<vk::Buffer>(passIn.drawCMDCountBuffer));
    setDef.update(set);
  }
  uint32_t offset = 0;
  for(int i = 0; i < passIn.drawGroupCount.size(); ++i) {
    drawCMDOffset->ptr<uint32_t>()[i] = offset;
    auto count = resources.get<uint32_t>(passIn.drawGroupCount[i]);
    offset += count;
  }
}
void ComputeCullDrawCMD::execute(RenderContext &ctx, Resources &resources) {
  auto total = resources.get<uint32_t>(passIn.meshInstancesCount);
  if(total == 0) return;
  auto maxCG = ctx.device.limits().maxComputeWorkGroupCount;
  auto totalGroup = uint32_t(std::ceil(total / double(local_size)));
  auto dx = std::min(totalGroup, maxCG[0]);
  totalGroup = uint32_t(totalGroup / double(dx));
  auto dy = std::min(std::max(totalGroup, 1u), maxCG[1]);
  totalGroup = uint32_t(totalGroup / double(dy));
  auto dz = std::min(std::max(totalGroup, 1u), maxCG[1]);

  auto cb = ctx.compute;

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

auto addComputeCullDrawCMDPass(
  FrameGraph &builder, const ComputeCullDrawCMD::PassIn &passIn)
  -> ComputeCullDrawCMD::PassOut {
  auto out = builder.addPass<ComputeCullDrawCMD>("ComputeCullDrawCMDPass", passIn);
  ComputeCullDrawCMD::PassOut passOut;
  passOut.drawCMDBuffer = out[0];
  passOut.drawCMDCountBuffer = out[1];
  return passOut;
}
}