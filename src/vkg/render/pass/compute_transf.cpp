#include "compute_transf.hpp"

#include <utility>
#include "deferred/comp/transform_comp.hpp"

namespace vkg {

ComputeTransf::ComputeTransf(ComputeTransf::PassIn in): in(std::move(in)) {}
void ComputeTransf::setup(PassBuilder &builder) {
  setDef.init(builder.device());
  pipeDef.transf(setDef);
  pipeDef.init(builder.device());
  descriptorPool = DescriptorPoolMaker().setLayout(setDef).createUnique(builder.device());

  pipe = ComputePipelineMaker(builder.device())
           .layout(pipeDef.layout())
           .shader(Shader{shader::deferred::comp::transform_comp_span, local_size, 1, 1})
           .createUnique();

  builder.read(in.transforms);
  builder.read(in.meshInstances);
  builder.read(in.meshInstancesCount);
  builder.write(in.matrices);
}
void ComputeTransf::compile(Resources &resources) {
  if(!set) {
    set = setDef.createSet(*descriptorPool);
    setDef.transforms(resources.get<vk::Buffer>(in.transforms));
    setDef.meshInstances(resources.get<vk::Buffer>(in.meshInstances));
    setDef.matrices(resources.get<vk::Buffer>(in.matrices));
    setDef.update(set);
  }
}
void ComputeTransf::execute(RenderContext &ctx, Resources &resources) {
  auto total = resources.get<uint32_t>(in.meshInstancesCount);
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
    resources.get<vk::Buffer>(in.matrices),
    0,
    VK_WHOLE_SIZE};
  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eAllCommands,
    {}, nullptr, barrier, nullptr);
  ctx.device.end(cb);
}

auto addComputeTransfPass(FrameGraph &builder, const ComputeTransf::PassIn &passIn)
  -> ComputeTransf::PassOut {
  auto out = builder.addPass<ComputeTransf>("ComputeTransfPass", passIn);
  ComputeTransf::PassOut passOut;
  passOut.matrices = out[0];
  return passOut;
}
}