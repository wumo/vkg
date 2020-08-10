#include "compute_transf.hpp"

#include "deferred/comp/transform_comp.hpp"

namespace vkg {

auto ComputeTransf::setup(PassBuilder &builder, const ComputeTransfPassIn &inputs)
  -> ComputeTransfPassOut {
  passIn = inputs;
  setDef.init(builder.device());
  pipeDef.transf(setDef);
  pipeDef.init(builder.device());
  descriptorPool = DescriptorPoolMaker().setLayout(setDef).createUnique(builder.device());

  pipe = ComputePipelineMaker(builder.device())
           .layout(pipeDef.layout())
           .shader(Shader{shader::deferred::comp::transform_comp_span, local_size, 1, 1})
           .createUnique();

  builder.read(passIn.transforms);
  builder.read(passIn.meshInstances);
  builder.read(passIn.meshInstancesCount);
  builder.read(passIn.maxNumMeshInstances);
  passOut.matrices = builder.create<vk::Buffer>("matrices");
  return passOut;
}
void ComputeTransf::compile(Resources &resources) {
  if(!set) {
    auto maxNumMeshInstances = resources.get(passIn.maxNumMeshInstances);
    matrices = buffer::devStorageBuffer(
      resources.device, sizeof(glm::mat4) * maxNumMeshInstances, name + "_matrices");
    set = setDef.createSet(*descriptorPool);
    setDef.transforms(resources.get(passIn.transforms));
    setDef.meshInstances(resources.get(passIn.meshInstances));
    setDef.matrices(matrices->buffer());
    setDef.update(set);
    resources.set(passOut.matrices, matrices->buffer());
  }
}
void ComputeTransf::execute(RenderContext &ctx, Resources &resources) {
  auto total = resources.get(passIn.meshInstancesCount);
  if(total == 0) return;
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
    vk::PipelineBindPoint::eCompute, pipeDef.layout(), pipeDef.transf.set(), set,
    nullptr);
  cb.pushConstants<uint32_t>(
    pipeDef.layout(), vk::ShaderStageFlagBits::eCompute, 0, total);
  cb.dispatch(dx, dy, dz);

  vk::BufferMemoryBarrier barrier{
    vk::AccessFlagBits::eShaderWrite,
    vk::AccessFlagBits::eShaderRead,
    VK_QUEUE_FAMILY_IGNORED,
    VK_QUEUE_FAMILY_IGNORED,
    matrices->buffer(),
    0,
    VK_WHOLE_SIZE};
  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader,
    vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eAllGraphics,
    {}, nullptr, barrier, nullptr);
  ctx.device.end(cb);
}

}