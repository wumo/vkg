#include "compute_transf.hpp"

#include "common/transform_comp.hpp"

namespace vkg {

void ComputeTransf::setup(PassBuilder &builder) {
  builder.read(passIn.transforms);
  builder.read(passIn.meshInstances);
  builder.read(passIn.meshInstancesCount);
  builder.read(passIn.sceneConfig);
  passOut = {
    .matrices = builder.create<BufferInfo>("matrices"),
  };
}
void ComputeTransf::compile(RenderContext &ctx, Resources &resources) {
  if(!init) {
    init = true;

    setDef.init(ctx.device);
    pipeDef.transf(setDef);
    pipeDef.init(ctx.device);

    pipe = ComputePipelineMaker(ctx.device)
             .layout(pipeDef.layout())
             .shader(Shader{shader::common::transform_comp_span, local_size, 1, 1})
             .createUnique();

    descriptorPool = DescriptorPoolMaker()
                       .pipelineLayout(pipeDef, ctx.numFrames)
                       .createUnique(ctx.device);

    auto sceneConfig = resources.get(passIn.sceneConfig);

    frames.resize(ctx.numFrames);
    for(auto i = 0u; i < ctx.numFrames; ++i) {
      auto &frame = frames[i];
      frame.matrices = buffer::devStorageBuffer(
        resources.device, sizeof(glm::mat4) * sceneConfig.maxNumMeshInstances,
        name + "_matrices");
      frame.set = setDef.createSet(*descriptorPool);
    }
  }
  auto &frame = frames[ctx.frameIndex];

  setDef.transforms(resources.get(passIn.transforms));
  setDef.meshInstances(resources.get(passIn.meshInstances));
  setDef.matrices(frame.matrices->bufferInfo());
  setDef.update(frame.set);

  resources.set(passOut.matrices, frame.matrices->bufferInfo());
}
void ComputeTransf::execute(RenderContext &ctx, Resources &resources) {
  auto total = resources.get(passIn.meshInstancesCount);
  if(total == 0) return;

  auto &frame = frames[ctx.frameIndex];

  auto maxCG = ctx.device.limits().maxComputeWorkGroupCount;
  auto totalGroup = uint32_t(std::ceil(total / double(local_size)));
  auto dx = std::min(totalGroup, maxCG[0]);
  totalGroup = uint32_t(totalGroup / double(dx));
  auto dy = std::min(std::max(totalGroup, 1u), maxCG[1]);
  totalGroup = uint32_t(totalGroup / double(dy));
  auto dz = std::min(std::max(totalGroup, 1u), maxCG[1]);

  auto cb = ctx.compute;
  ctx.device.begin(cb, "compute transform");
  cb.bindPipeline(vk::PipelineBindPoint::eCompute, *pipe);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eCompute, pipeDef.layout(), pipeDef.transf.set(), frame.set,
    nullptr);
  pushConstant = {total};
  cb.pushConstants<PushConstant>(
    pipeDef.layout(), vk::ShaderStageFlagBits::eCompute, 0, pushConstant);
  cb.dispatch(dx, dy, dz);

  auto bufInfo = frame.matrices->bufferInfo();
  vk::BufferMemoryBarrier barrier{
    vk::AccessFlagBits::eShaderWrite,
    vk::AccessFlagBits::eShaderRead,
    VK_QUEUE_FAMILY_IGNORED,
    VK_QUEUE_FAMILY_IGNORED,
    bufInfo.buffer,
    bufInfo.offset,
    bufInfo.size};
  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader,
    {}, nullptr, barrier, nullptr);
  ctx.device.end(cb);
}

}