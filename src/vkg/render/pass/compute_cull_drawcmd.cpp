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

  builder.read(passIn);
  passOut = {
    .drawInfos = builder.create<DrawInfos>("drawInfos"),
  };
  return passOut;
}
void ComputeCullDrawCMD::compile(RenderContext &ctx, Resources &resources) {
  auto frustums = resources.get(passIn.frustums);
  auto sceneConfig = resources.get(passIn.sceneConfig);
  auto maxPerGroup = resources.get(passIn.maxPerGroup);
  if(!init) {
    init = true;
    set = setDef.createSet(*descriptorPool);

    numFrustums = uint32_t(frustums.size());
    numDrawCMDsPerFrustum = sceneConfig.maxNumMeshInstances;
    numDrawGroups = uint32_t(maxPerGroup.size());

    cmdOffsetPerGroup.resize(numDrawGroups);

    frustumsBuf = buffer::devStorageBuffer(
      resources.device, sizeof(Frustum) * numFrustums * ctx.numFrames, "frustum");
    drawCMD = buffer::devIndirectStorageBuffer(
      resources.device,
      sizeof(vk::DrawIndexedIndirectCommand) * numDrawCMDsPerFrustum * numFrustums *
        ctx.numFrames,
      "drawCMD");
    cmdOffsetPerGroupBuffer = buffer::devStorageBuffer(
      resources.device, sizeof(uint32_t) * numDrawGroups * numFrustums * ctx.numFrames,
      "drawCMDOffset");
    countPerGroupBuffer = buffer::devIndirectStorageBuffer(
      resources.device, sizeof(uint32_t) * numDrawGroups * numFrustums * ctx.numFrames,
      name + "drawGroupCount");

    setDef.frustums(frustumsBuf->bufferInfo());
    setDef.meshInstances(resources.get(passIn.meshInstances));
    setDef.primitives(resources.get(passIn.primitives));
    setDef.matrices(resources.get(passIn.matrices));
    setDef.drawCMD(drawCMD->bufferInfo());
    setDef.cmdOffsetPerGroup(cmdOffsetPerGroupBuffer->bufferInfo());
    setDef.drawCMDCount(countPerGroupBuffer->bufferInfo());
    setDef.update(set);
  }
  errorIf(frustums.size() != numFrustums, "number of frustums changed!");
  errorIf(maxPerGroup.size() != numDrawGroups, "number of draw group changed!");

  uint32_t offset = 0;
  for(int i = 0; i < numDrawGroups; ++i) {
    cmdOffsetPerGroup[i] = offset;
    offset += maxPerGroup[i];
  }
  countOffset = ctx.frameIndex * numDrawGroups * numFrustums;

  DrawInfos drawInfos;
  drawInfos.drawInfo.resize(numFrustums);

  offset = ctx.frameIndex * numDrawCMDsPerFrustum * numFrustums;
  for(int f = 0; f < numFrustums; ++f) {
    drawInfos.drawInfo[f].resize(numDrawGroups);

    DrawInfo drawInfo;
    for(int g = 0; g < numDrawGroups; ++g) {
      drawInfo.drawCMD = {
        drawCMD->bufferInfo().buffer,
        drawCMD->bufferInfo().offset +
          sizeof(vk::DrawIndexedIndirectCommand) * (offset + cmdOffsetPerGroup[g])};
      drawInfo.drawCMDCount = {
        countPerGroupBuffer->bufferInfo().buffer,
        countPerGroupBuffer->bufferInfo().offset + sizeof(uint32_t) * (countOffset + g)};
      drawInfo.maxCount = maxPerGroup[g];
      drawInfos.drawInfo[f][g] = drawInfo;
    }
    offset += numDrawCMDsPerFrustum;
  }
  resources.set(passOut.drawInfos, drawInfos);
}
void ComputeCullDrawCMD::execute(RenderContext &ctx, Resources &resources) {
  auto totalMeshInstances = resources.get(passIn.meshInstancesCount);
  if(totalMeshInstances == 0) return;

  auto cb = ctx.compute;

  auto frustums = resources.get(passIn.frustums);
  ctx.device.begin(cb, "update frustums");
  cb.updateBuffer(
    frustumsBuf->bufferInfo().buffer, ctx.frameIndex * frustums.size_bytes(),
    frustums.size_bytes(), frustums.data());

  cb.updateBuffer(
    cmdOffsetPerGroupBuffer->bufferInfo().buffer, sizeof(uint32_t) * countOffset,
    sizeof(uint32_t) * cmdOffsetPerGroup.size(), cmdOffsetPerGroup.data());

  /**
       * TODO It seems that we have to use cb.fillBuffer to initialize Dev.drawCMDCount instead
       * of memset the host persistent mapped buffer pointer, otherwise atomicAdd operation in
       * shader will not work as expected. Need to fully inspect the real reason of this. And
       * this may invalidate other host coherent memories that will be accessed by compute shader.
       */
  cb.fillBuffer(
    countPerGroupBuffer->bufferInfo().buffer, sizeof(uint32_t) * countOffset,
    sizeof(uint32_t) * numDrawGroups, 0u);
  ctx.device.end(cb);

  pushConstant = {
    .totalFrustums = numFrustums,
    .totalMeshInstances = totalMeshInstances,
    .cmdFrameStride = numDrawCMDsPerFrustum * numFrustums,
    .cmdFrustumStride = numDrawCMDsPerFrustum,
    .groupFrameStride = numDrawGroups * numFrustums,
    .transformStride = resources.get(passIn.transformStride),
    .frame = ctx.frameIndex};

  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eAllCommands, {},
    vk::MemoryBarrier{
      vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead},
    nullptr, nullptr);

  auto totalDispatch = totalMeshInstances * numFrustums;
  auto maxCG = ctx.device.limits().maxComputeWorkGroupCount;
  auto totalGroup = uint32_t(std::ceil(totalDispatch / double(local_size)));
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