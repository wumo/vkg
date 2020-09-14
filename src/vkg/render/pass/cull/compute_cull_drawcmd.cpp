#include "compute_cull_drawcmd.hpp"

#include <utility>
#include "common/cull_draw_group_comp.hpp"

namespace vkg {
ComputeCullDrawCMD::ComputeCullDrawCMD(std::set<ShadeModel> allowedShadeModel)
  : allowedShadeModel(std::move(allowedShadeModel)) {}
void ComputeCullDrawCMD::setup(PassBuilder &builder) {
  builder.read(passIn);
  passOut = {
    .drawCMDs = builder.create<DrawInfos>("drawCMDs"),
  };
}
void ComputeCullDrawCMD::compile(RenderContext &ctx, Resources &resources) {
  auto frustums = resources.get(passIn.frustums);
  auto sceneConfig = resources.get(passIn.sceneConfig);
  auto maxPerGroup = resources.get(passIn.maxPerShadeModel);
  if(!init) {
    init = true;

    setDef.init(ctx.device);
    pipeDef.transf(setDef);
    pipeDef.init(ctx.device);
    pipe = ComputePipelineMaker(ctx.device)
             .layout(pipeDef.layout())
             .shader(Shader{shader::common::cull_draw_group_comp_span, local_size, 1, 1})
             .createUnique();

    descriptorPool = DescriptorPoolMaker()
                       .pipelineLayout(pipeDef, ctx.numFrames)
                       .createUnique(ctx.device);

    frames.resize(ctx.numFrames);

    numFrustums = uint32_t(frustums.size());
    numDrawCMDsPerFrustum = sceneConfig.maxNumMeshInstances;
    numShadeModels = uint32_t(maxPerGroup.size());

    cmdOffsetOfShadeModelInFrustum.resize(numShadeModels);

    {
      std::vector<VkBool32> allowedGroup_(numShadeModels);
      for(auto g = 0u; g < numShadeModels; ++g)
        allowedGroup_[g] = allowedShadeModel.contains(static_cast<ShadeModel>(g));

      allowedShadeModelBuf = buffer::devStorageBuffer(
        resources.device, allowedGroup_.size() * sizeof(VkBool32),
        toString(name, "_allowedGroup"));
      buffer::uploadVec(ctx.frameIndex, *allowedShadeModelBuf, allowedGroup_);
    }

    for(int i = 0; i < ctx.numFrames; ++i) {
      auto &frame = frames[i];
      frame.set = setDef.createSet(*descriptorPool);

      frame.frustumsBuf = buffer::devStorageBuffer(
        resources.device, sizeof(Frustum) * numFrustums, toString(name, "_frustum_", i));
      frame.drawCMD = buffer::devIndirectStorageBuffer(
        resources.device,
        sizeof(vk::DrawIndexedIndirectCommand) * numDrawCMDsPerFrustum * numFrustums,
        toString(name, "_drawCMD_", i));
      frame.cmdOffsetPerShadeModelBuffer = buffer::devStorageBuffer(
        resources.device, sizeof(uint32_t) * numShadeModels,
        toString(name, "_drawCMDOffset_", i));
      frame.countOfShadeModelBuffer = buffer::devIndirectStorageBuffer(
        resources.device, sizeof(uint32_t) * numShadeModels * numFrustums,
        toString(name, "_drawGroupCount_", i));
    }
  }
  errorIf(frustums.size() != numFrustums, "number of frustums changed!");
  errorIf(maxPerGroup.size() != numShadeModels, "number of draw group changed!");

  auto &frame = frames[ctx.frameIndex];

  setDef.frustums(frame.frustumsBuf->bufferInfo());
  setDef.meshInstances(resources.get(passIn.meshInstances));
  setDef.primitives(resources.get(passIn.primitives));
  setDef.matrices(resources.get(passIn.matrices));
  setDef.drawCMD(frame.drawCMD->bufferInfo());
  setDef.cmdOffsetPerGroup(frame.cmdOffsetPerShadeModelBuffer->bufferInfo());
  setDef.drawCMDCount(frame.countOfShadeModelBuffer->bufferInfo());
  setDef.allowedShadeModel(allowedShadeModelBuf->bufferInfo());
  setDef.update(frame.set);

  {
    uint32_t offset = 0;
    for(int i = 0; i < numShadeModels; ++i) {
      cmdOffsetOfShadeModelInFrustum[i] = offset;
      offset += maxPerGroup[i];
    }
  }

  DrawInfos drawInfos;
  drawInfos.cmdsPerShadeModel.resize(numFrustums);

  auto drawCMDBufInfo = frame.drawCMD->bufferInfo();
  auto countOfGroupBufInfo = frame.countOfShadeModelBuffer->bufferInfo();
  for(int f = 0; f < numFrustums; ++f) {
    drawInfos.cmdsPerShadeModel[f].resize(numShadeModels);
    DrawInfo drawInfo;
    for(int g = 0; g < numShadeModels; ++g) {
      drawInfo.cmdBuf = {
        drawCMDBufInfo.buffer,
        drawCMDBufInfo.offset +
          sizeof(vk::DrawIndexedIndirectCommand) *
            (f * numDrawCMDsPerFrustum + cmdOffsetOfShadeModelInFrustum[g])};
      drawInfo.countBuf = {
        countOfGroupBufInfo.buffer,
        countOfGroupBufInfo.offset + sizeof(uint32_t) * (f * numShadeModels + g)};
      drawInfo.maxCount = maxPerGroup[g];
      drawInfos.cmdsPerShadeModel[f][g] = drawInfo;
    }
  }
  resources.set(passOut.drawCMDs, drawInfos);
}
void ComputeCullDrawCMD::execute(RenderContext &ctx, Resources &resources) {
  auto totalMeshInstances = resources.get(passIn.meshInstancesCount);
  if(totalMeshInstances == 0) return;

  auto &frame = frames[ctx.frameIndex];

  auto cb = ctx.cb;

  ctx.device.begin(cb, "update frustums");

  auto frustums = resources.get(passIn.frustums);
  auto bufInfo = frame.frustumsBuf->bufferInfo();
  cb.updateBuffer(bufInfo.buffer, bufInfo.offset, frustums.size_bytes(), frustums.data());

  bufInfo = frame.cmdOffsetPerShadeModelBuffer->bufferInfo();
  cb.updateBuffer(
    bufInfo.buffer, bufInfo.offset, sizeof(uint32_t) * cmdOffsetOfShadeModelInFrustum.size(),
    cmdOffsetOfShadeModelInFrustum.data());

  bufInfo = frame.countOfShadeModelBuffer->bufferInfo();
  cb.fillBuffer(
    bufInfo.buffer, bufInfo.offset, sizeof(uint32_t) * numFrustums * numShadeModels, 0u);

  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader, {},
    vk::MemoryBarrier{
      vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderWrite},
    nullptr, nullptr);
  ctx.device.end(cb);

  auto totalDispatch = totalMeshInstances * numFrustums;
  auto maxCG = ctx.device.limits().maxComputeWorkGroupCount;
  auto totalGroup = uint32_t(std::ceil(totalDispatch / double(local_size)));
  auto dx = std::min(totalGroup, maxCG[0]);
  totalGroup = uint32_t(totalGroup / double(dx));
  auto dy = std::min(std::max(totalGroup, 1u), maxCG[1]);
  totalGroup = uint32_t(totalGroup / double(dy));
  auto dz = std::min(std::max(totalGroup, 1u), maxCG[2]);

  ctx.device.begin(cb, name + " compute cull drawGroup");
  cb.bindPipeline(vk::PipelineBindPoint::eCompute, *pipe);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eCompute, pipeDef.layout(), pipeDef.transf.set(), frame.set,
    nullptr);
  pushConstant = {
    .totalFrustums = numFrustums,
    .totalMeshInstances = totalMeshInstances,
    .cmdFrustumStride = numDrawCMDsPerFrustum,
    .groupStride = numShadeModels,
    .frame = ctx.frameIndex,
  };
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