#include "deferred.hpp"

namespace vkg {
void DeferredPass::execute(RenderContext &ctx, Resources &resources) {
  auto drawCMDBuffer = resources.get(cullPassOut.drawCMDBuffer);
  auto drawCMDCountBuffer = resources.get(cullPassOut.drawCMDCountBuffer);
  auto drawCMDOffsets = resources.get(cullPassOut.drawCMDOffsets);
  auto drawGroupCount = resources.get(passIn.drawGroupCount);

  auto cb = ctx.graphics;
  image::transitTo(
    cb, *backImg_, vk::ImageLayout::eColorAttachmentOptimal,
    vk::AccessFlagBits::eColorAttachmentWrite,
    vk::PipelineStageFlagBits::eColorAttachmentOutput);

  std::array<vk::ClearValue, 7> clearValues{
    vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 0.0f}},
    vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 0.0f}},
    vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 0.0f}},
    vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 0.0f}},
    vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 0.0f}},
    vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 0.0f}},
    vk::ClearDepthStencilValue{1.0f, 0},
  };

  vk::RenderPassBeginInfo renderPassBeginInfo{
    *renderPass, *framebuffer,
    vk::Rect2D{{0, 0}, {backImg_->extent().width, backImg_->extent().height}},
    uint32_t(clearValues.size()), clearValues.data()};
  cb.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
  vk::Viewport viewport{
    0, 0, float(backImg_->extent().width), float(backImg_->extent().height), 0.0f, 1.0f};
  cb.setViewport(0, viewport);
  vk::Rect2D scissor{{0, 0}, {backImg_->extent().width, backImg_->extent().height}};
  cb.setScissor(0, scissor);

  auto &dev = resources.device;
  vk::DeviceSize zero{0};
  auto stride = sizeof(vk::DrawIndexedIndirectCommand);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics, deferredPipeDef.layout(),
    deferredPipeDef.scene.set(), sceneSet, nullptr);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics, deferredPipeDef.layout(),
    deferredPipeDef.gbuffer.set(), gbSet, nullptr);

  cb.bindVertexBuffers(0, resources.get(passIn.positions), zero);
  cb.bindVertexBuffers(1, resources.get(passIn.normals), zero);
  cb.bindVertexBuffers(2, resources.get(passIn.uvs), zero);
  cb.bindIndexBuffer(resources.get(passIn.indices), zero, vk::IndexType::eUint32);

  dev.begin(cb, "Subpass gbuffer triangles");
  if(wireframe_) cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *gbWireFramePipe);
  else
    cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *gbTriPipe);
  cb.drawIndexedIndirectCountKHR(
    drawCMDBuffer, stride * drawCMDOffsets[0], drawCMDCountBuffer, sizeof(uint32_t) * 0,
    drawGroupCount[0], stride);
  dev.end(cb);

  dev.begin(cb, "Subpass gbuffer lines");
  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *gbLinePipe);
  cb.drawIndexedIndirectCountKHR(
    drawCMDBuffer, stride * drawCMDOffsets[1], drawCMDCountBuffer, sizeof(uint32_t) * 1,
    drawGroupCount[1], stride);
  dev.end(cb);

  cb.nextSubpass(vk::SubpassContents::eInline);

  dev.begin(cb, "Subpass deferred lighting");
  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *litPipe);
  cb.draw(3, 1, 0, 0);
  dev.end(cb);

  cb.nextSubpass(vk::SubpassContents::eInline);

  dev.begin(cb, "Subpass unlit triangles");
  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *unlitTriPipe);
  cb.drawIndexedIndirectCountKHR(
    drawCMDBuffer, stride * drawCMDOffsets[2], drawCMDCountBuffer, sizeof(uint32_t) * 2,
    drawGroupCount[2], stride);
  dev.end(cb);

  dev.begin(cb, "Subpass unlit lines");
  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *unlitLinePipe);
  cb.drawIndexedIndirectCountKHR(
    drawCMDBuffer, stride * drawCMDOffsets[3], drawCMDCountBuffer, sizeof(uint32_t) * 3,
    drawGroupCount[3], stride);
  dev.end(cb);

  cb.nextSubpass(vk::SubpassContents::eInline);

  dev.begin(cb, "Subpass transparent triangles");
  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *transTriPipe);
  cb.drawIndexedIndirectCountKHR(
    drawCMDBuffer, stride * drawCMDOffsets[4], drawCMDCountBuffer, sizeof(uint32_t) * 4,
    drawGroupCount[4], stride);
  dev.end(cb);

  dev.begin(cb, "Subpass transparent lines");
  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *transLinePipe);
  cb.drawIndexedIndirectCountKHR(
    drawCMDBuffer, stride * drawCMDOffsets[5], drawCMDCountBuffer, sizeof(uint32_t) * 5,
    drawGroupCount[5], stride);
  dev.end(cb);

  cb.endRenderPass();
}
}