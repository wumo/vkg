#include "cam_frustum_pass.hpp"
namespace vkg {

void CamFrustumPass::setup(PassBuilder &builder) {
  builder.read(passIn.camera);
  passOut = {
    .camFrustum = builder.create<std::span<Frustum>>("camFrustum"),
    .camBuffer = builder.create<BufferInfo>("camBuffer"),
  };
}
void CamFrustumPass::compile(RenderContext &ctx, Resources &resources) {
  if(!init) {
    init = true;
    frustums.resize(1);
    camBuffers.resize(ctx.numFrames);
    for(int i = 0; i < ctx.numFrames; ++i) {
      camBuffers[i] = buffer::devStorageBuffer(
        resources.device, sizeof(Camera::Desc) * ctx.numFrames,
        toString("camBuffer_", i));
    }
  }
  auto *camera = resources.get(passIn.camera);
  frustums[0] = Frustum{camera->proj() * camera->view()};

  resources.set(passOut.camFrustum, {frustums});
  resources.set(passOut.camBuffer, camBuffers[ctx.frameIndex]->bufferInfo());
}
void CamFrustumPass::execute(RenderContext &ctx, Resources &resources) {
  auto *camera = resources.get(passIn.camera);
  auto desc = camera->desc();
  desc.frame = ctx.frameIndex;
  auto bufInfo = camBuffers[ctx.frameIndex]->bufferInfo();
  auto cb = ctx.cb;
  ctx.device.begin(cb, "update camera");
  cb.updateBuffer(bufInfo.buffer, bufInfo.offset, sizeof(desc), &desc);
  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eAllCommands, {},
    vk::MemoryBarrier{
      vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead},
    nullptr, nullptr);
  ctx.device.end(cb);
}
}