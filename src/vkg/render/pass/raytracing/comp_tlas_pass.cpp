#include "comp_tlas_pass.hpp"
#include "raytracing/comp/tlas_comp.hpp"
#include "vkg/render/draw_group.hpp"

namespace vkg {
void CompTLASPass::setup(PassBuilder &builder) {
  builder.read(passIn);
  passOut = {
    .tlasCount = builder.create<uint32_t>("tlasInstanceCount"),
    .tlas = builder.create<BufferInfo>("tlasInstances"),
  };
}
void CompTLASPass::compile(RenderContext &ctx, Resources &resources) {
  if(!init) {
    init = true;

    setDef.init(ctx.device);
    pipeDef.set(setDef);
    pipeDef.init(ctx.device);
    pipe = ComputePipelineMaker(ctx.device)
             .layout(pipeDef.layout())
             .shader(Shader{shader::raytracing::comp::tlas_comp_span, local_size, 1, 1})
             .createUnique();

    descriptorPool = DescriptorPoolMaker()
                       .pipelineLayout(pipeDef, ctx.numFrames)
                       .createUnique(ctx.device);

    auto sceneConfig = resources.get(passIn.sceneConfig);

    frames.resize(ctx.numFrames);
    for(int i = 0; i < ctx.numFrames; ++i) {
      auto &frame = frames[i];
      frame.set = setDef.createSet(*descriptorPool);

      frame.tlasInstanceCount = buffer::devStorageBuffer(
        resources.device, sizeof(uint32_t), toString(name, "_tlasInstanceCount_", i));
      frame.tlasInstances = buffer::devBuffer(
        resources.device,
        vk::BufferUsageFlagBits::eRayTracingNV | vk::BufferUsageFlagBits::eStorageBuffer,
        sizeof(VkAccelerationStructureInstanceKHR) * sceneConfig.maxNumMeshInstances,
        toString(name, "_tlasInstances_", i));
    }
  }
  auto &frame = frames[ctx.frameIndex];

  setDef.meshInstances(resources.get(passIn.meshInstances));
  setDef.primitives(resources.get(passIn.primitives));
  setDef.matrices(resources.get(passIn.matrices));
  setDef.tlasInstanceCount(frame.tlasInstanceCount->bufferInfo());
  setDef.tlasInstances(frame.tlasInstances->bufferInfo());
  setDef.update(frame.set);

  auto countPerDrawGroup = resources.get(passIn.countPerDrawGroup);
  auto tlasCount = countPerDrawGroup[value(DrawGroup::BRDF)] +
                   countPerDrawGroup[value(DrawGroup::Reflective)] +
                   countPerDrawGroup[value(DrawGroup::Refractive)];
  resources.set(passOut.tlasCount, tlasCount);
  resources.set(passOut.tlas, frame.tlasInstances->bufferInfo());
}
void CompTLASPass::execute(RenderContext &ctx, Resources &resources) {
  auto meshInstancesCount = resources.get(passIn.meshInstancesCount);
  if(meshInstancesCount == 0) return;

  auto &frame = frames[ctx.frameIndex];

  auto cb = ctx.cb;

  ctx.device.begin(cb, "compute tlas");

  auto bufInfo = frame.tlasInstanceCount->bufferInfo();
  cb.fillBuffer(bufInfo.buffer, bufInfo.offset, sizeof(uint32_t), 0u);
  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader, {},
    vk::MemoryBarrier{
      vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead},
    nullptr, nullptr);

  auto totalDispatch = meshInstancesCount;
  auto maxCG = ctx.device.limits().maxComputeWorkGroupCount;
  auto totalGroup = uint32_t(std::ceil(totalDispatch / double(local_size)));
  auto dx = std::min(totalGroup, maxCG[0]);
  totalGroup = uint32_t(totalGroup / double(dx));
  auto dy = std::min(std::max(totalGroup, 1u), maxCG[1]);
  totalGroup = uint32_t(totalGroup / double(dy));
  auto dz = std::min(std::max(totalGroup, 1u), maxCG[2]);

  cb.bindPipeline(vk::PipelineBindPoint::eCompute, *pipe);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eCompute, pipeDef.layout(), pipeDef.set.set(), frame.set,
    nullptr);
  pushConstant = {
    .totalMeshInstances = meshInstancesCount,
    .frame = ctx.frameIndex,
  };
  cb.pushConstants<PushConstant>(
    pipeDef.layout(), vk::ShaderStageFlagBits::eCompute, 0, pushConstant);
  cb.dispatch(dx, dy, dz);

  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader,
    vk::PipelineStageFlagBits::eAccelerationStructureBuildNV, {},
    vk::MemoryBarrier{
      vk::AccessFlagBits::eShaderWrite,
      vk::AccessFlagBits::eAccelerationStructureWriteNV},
    nullptr, nullptr);
  ctx.device.end(cb);
}
}