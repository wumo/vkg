#include "compute_cull_drawcmd.hpp"
#include "common/cull_draw_group_comp.hpp"

namespace vkg {

auto ComputeCullDrawCMD::setup(
  PassBuilder &builder, const ComputeCullDrawCMDPassIn &inputs)
  -> ComputeCullDrawCMDPassOut {
  passIn = inputs;

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
  builder.read(passIn.sceneConfig);
  builder.read(passIn.primitives);
  builder.read(passIn.matrices);
  builder.read(passIn.drawGroupCount);
  passOut.drawCMDBuffer = builder.create<vk::Buffer>("drawCMDBuffer");
  passOut.drawCMDCountBuffer = builder.create<vk::Buffer>("drawCMDCountBuffer");
  passOut.drawCMDOffsets = builder.create<std::vector<uint32_t>>("drawCMDOffsets");
  passOut.drawCMDCountOffset = builder.create<uint32_t>("drawCMDCountOffset");

  return passOut;
}
void ComputeCullDrawCMD::compile(RenderContext &ctx, Resources &resources) {
  if(!init) {
    init = true;
    auto sceneConfig = resources.get(passIn.sceneConfig);
    auto drawGroupCount = resources.get(passIn.drawGroupCount);
    numDrawCMDs = sceneConfig.maxNumMeshInstances;
    numDrawCMDCounts = uint32_t(drawGroupCount.size());
    drawCMD = buffer::devIndirectStorageBuffer(
      resources.device,
      sizeof(vk::DrawIndexedIndirectCommand) * numDrawCMDs * ctx.numFrames, "drawCMD");
    drawCMDOffsetBuffer = buffer::devStorageBuffer(
      resources.device, sizeof(uint32_t) * numDrawCMDCounts * ctx.numFrames,
      "drawCMDOffset");
    drawCMDCount = buffer::devIndirectStorageBuffer(
      resources.device, sizeof(uint32_t) * numDrawCMDCounts * ctx.numFrames,
      name + "_drawCMDCount");

    resources.set(passOut.drawCMDBuffer, drawCMD->buffer());
    resources.set(passOut.drawCMDCountBuffer, drawCMDCount->buffer());

    set = setDef.createSet(*descriptorPool);
    setDef.frustum(resources.get(passIn.frustum));
    setDef.meshInstances(resources.get(passIn.meshInstances));
    setDef.primitives(resources.get(passIn.primitives));
    setDef.matrices(resources.get(passIn.matrices));
    setDef.drawCMD(drawCMD->buffer());
    setDef.drawCMDOffset(drawCMDOffsetBuffer->buffer());
    setDef.drawCMDCount(drawCMDCount->buffer());
    setDef.update(set);
  }
  auto drawGroupCount = resources.get(passIn.drawGroupCount);
  assert(drawGroupCount.size() == numDrawCMDCounts);
  std::vector<uint32_t> offsets(numDrawCMDCounts);
  uint32_t offset = ctx.frameIndex * numDrawCMDs;
  for(int i = 0; i < drawGroupCount.size(); ++i) {
    offsets[i] = offset;
    offset += drawGroupCount[i];
  }
  resources.set(passOut.drawCMDOffsets, offsets);
  resources.set(passOut.drawCMDCountOffset, ctx.frameIndex * numDrawCMDCounts);
}
void ComputeCullDrawCMD::execute(RenderContext &ctx, Resources &resources) {
  auto total = resources.get(passIn.meshInstancesCount);
  if(total == 0) return;

  auto cb = ctx.compute;

  std::vector<uint32_t> offsets = resources.get(passOut.drawCMDOffsets);
  uint32_t drawCMDCountOffset = resources.get(passOut.drawCMDCountOffset);
  cb.updateBuffer(
    drawCMDOffsetBuffer->buffer(), sizeof(uint32_t) * drawCMDCountOffset,
    sizeof(uint32_t) * offsets.size(), offsets.data());

  /**
       * TODO It seems that we have to use cb.fillBuffer to initialize Dev.drawCMDCount instead
       * of memset the host persistent mapped buffer pointer, otherwise atomicAdd operation in
       * shader will not work as expected. Need to fully inspect the real reason of this. And
       * this may invalidate other host coherent memories that will be accessed by compute shader.
       */
  cb.fillBuffer(
    drawCMDCount->buffer(), sizeof(uint32_t) * drawCMDCountOffset,
    sizeof(uint32_t) * numDrawCMDCounts, 0u);

  pushConstant.totalMeshInstances = total;
  pushConstant.drawCMDCountOffset = drawCMDCountOffset;

  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eAllCommands, {},
    vk::MemoryBarrier{
      vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead},
    nullptr, nullptr);

  auto maxCG = ctx.device.limits().maxComputeWorkGroupCount;
  auto totalGroup = uint32_t(std::ceil(total / double(local_size)));
  auto dx = std::min(totalGroup, maxCG[0]);
  totalGroup = uint32_t(totalGroup / double(dx));
  auto dy = std::min(std::max(totalGroup, 1u), maxCG[1]);
  totalGroup = uint32_t(totalGroup / double(dy));
  auto dz = std::min(std::max(totalGroup, 1u), maxCG[1]);

  ctx.device.begin(cb, name + " compute cull drawGroup");
  cb.bindPipeline(vk::PipelineBindPoint::eCompute, *pipe);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eCompute, pipeDef.layout(), pipeDef.transf.set(), set,
    nullptr);
  cb.pushConstants<PushConstant>(
    pipeDef.layout(), vk::ShaderStageFlagBits::eCompute, 0, pushConstant);
  cb.dispatch(dx, dy, dz);

  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eDrawIndirect,
    {},
    vk::MemoryBarrier{
      vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eIndirectCommandRead},
    nullptr, nullptr);
  ctx.device.end(cb);
}

}