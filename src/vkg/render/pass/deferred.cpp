#include "deferred.hpp"

#include <utility>
#include "vkg/math/frustum.hpp"
#include "compute_cull_drawcmd.hpp"
#include "vkg/render/model/camera.hpp"

namespace vkg {

auto DeferredPass::setup(PassBuilder &builder, const DeferredPassIn &inputs)
  -> DeferredPassOut {
  passIn = inputs;

  builder.read(passIn.camera);
  builder.read(passIn.maxNumMeshInstances);
  builder.read(passIn.drawGroupCount);
  passOut.camFrustum = builder.create("_camFrustum", ResourceType::eBuffer);
  passOut.drawCMDBuffer = builder.create("_drawCMDBuffer", ResourceType::eBuffer);
  passOut.drawCMDCountBuffer =
    builder.create("_drawCMDCountBuffer", ResourceType::eBuffer);

  auto cullCMDOut =
    builder
      .newPass<ComputeCullDrawCMD, ComputeCullDrawCMDPassIn, ComputeCullDrawCMDPassOut>(
        "ComputeCullDrawCMD",
        {passOut.camFrustum, passIn.meshInstances, passIn.meshInstancesCount,
         passIn.primitives, passIn.matrices, passOut.drawCMDBuffer,
         passOut.drawCMDCountBuffer, passIn.drawGroupCount});

  return passOut;
}
void DeferredPass::compile(Resources &resources) {
  if(!init) {
    init = true;

    auto maxNumMeshInstances = resources.get<uint32_t>(passIn.maxNumMeshInstances);
    auto drawGroupCount = resources.get<std::vector<uint32_t>>(passIn.drawGroupCount);
    camFrustum =
      buffer::hostUniformBuffer(resources.device, sizeof(Frustum), name + "_camFrustum");
    drawCMD = buffer::devIndirectStorageBuffer(
      resources.device, sizeof(vk::DrawIndexedIndirectCommand) * maxNumMeshInstances,
      "drawCMD");
    drawCMDCount = buffer::devIndirectStorageBuffer(
      resources.device, sizeof(uint32_t) * drawGroupCount.size(), name + "_drawCMDCount");
  }
  auto *camera = resources.get<Camera *>(passIn.camera);
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
