#include "deferred.hpp"

#include <utility>
#include "vkg/math/frustum.hpp"
#include "compute_cull_drawcmd.hpp"

namespace vkg {

auto DeferredPass::setup(PassBuilder &builder, const DeferredPassIn &inputs)
  -> DeferredPassOut {
  passIn = inputs;

  builder.read(passIn.swapchainExtent);
  builder.read(passIn.camera);
  builder.read(passIn.maxNumMeshInstances);
  builder.read(passIn.drawGroupCount);
  passOut.camFrustum = builder.create<vk::Buffer>("camFrustum");
  passOut.drawCMDBuffer = builder.create<vk::Buffer>("drawCMDBuffer");
  passOut.drawCMDCountBuffer = builder.create<vk::Buffer>("drawCMDCountBuffer");

  auto cullCMDOut = builder.newPass<ComputeCullDrawCMD>(
    "Cull", ComputeCullDrawCMDPassIn{
              passOut.camFrustum, passIn.meshInstances, passIn.meshInstancesCount,
              passIn.primitives, passIn.matrices, passOut.drawCMDBuffer,
              passOut.drawCMDCountBuffer, passIn.drawGroupCount});

  return passOut;
}
void DeferredPass::compile(Resources &resources) {
  auto extent = resources.get(passIn.swapchainExtent);
  if(!init) {
    init = true;

    auto maxNumMeshInstances = resources.get(passIn.maxNumMeshInstances);
    auto drawGroupCount = resources.get(passIn.drawGroupCount);
    camFrustum =
      buffer::hostUniformBuffer(resources.device, sizeof(Frustum), name + "_camFrustum");
    drawCMD = buffer::devIndirectStorageBuffer(
      resources.device, sizeof(vk::DrawIndexedIndirectCommand) * maxNumMeshInstances,
      "drawCMD");
    drawCMDCount = buffer::devIndirectStorageBuffer(
      resources.device, sizeof(uint32_t) * drawGroupCount.size(), name + "_drawCMDCount");
    resources.set(passOut.drawCMDBuffer, drawCMD->buffer());
    resources.set(passOut.drawCMDCountBuffer, drawCMDCount->buffer());
    resources.set(passOut.camFrustum, camFrustum->buffer());
  }
  auto *camera = resources.get(passIn.camera);
  *camFrustum->ptr<Frustum>() = Frustum{camera->proj() * camera->view()};
}
void DeferredPass::execute(RenderContext &ctx, Resources &resources) {
  auto cb = ctx.compute;

  /**
     * TODO It seems that we have to use cb.fillBuffer to initialize Dev.drawCMDCount instead
     * of memset the host persistent mapped buffer pointer, otherwise atomicAdd operation in
     * shader will not work as expected. Need to fully inspect the real reason of this. And
     * this may invalidate other host coherent memories that will be accessed by compute shader.
     */
  cb.fillBuffer(drawCMDCount->buffer(), 0, VK_WHOLE_SIZE, 0u);

  vk::MemoryBarrier barrier{
    vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead};
  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader, {},
    barrier, nullptr, nullptr);
}

}
