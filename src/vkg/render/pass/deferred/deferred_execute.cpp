#include "deferred.hpp"

namespace vkg {
void DeferredPass::execute(RenderContext &ctx, Resources &resources) {
  auto drawInfos = resources.get(cullPassOut.drawInfos);
  auto atmosSetting = resources.get(passIn.atmosSetting);
  auto shadowMapSetting = resources.get(passIn.shadowMapSetting);

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
  cb.pushConstants<uint32_t>(
    deferredPipeDef.layout(),
    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
    ctx.frameIndex);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics, deferredPipeDef.layout(),
    deferredPipeDef.scene.set(), sceneSet, nullptr);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics, deferredPipeDef.layout(),
    deferredPipeDef.gbuffer.set(), gbSet, nullptr);
  if(atmosSetting.isEnabled())
    cb.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, deferredPipeDef.layout(),
      deferredPipeDef.atmosphere.set(), atmosphereSet, nullptr);
  if(shadowMapSetting.isEnabled())
    cb.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, deferredPipeDef.layout(),
      deferredPipeDef.shadowMap.set(), shadowMapSet, nullptr);

  auto bufInfo = resources.get(passIn.positions);
  cb.bindVertexBuffers(0, bufInfo.buffer, bufInfo.offset);
  bufInfo = resources.get(passIn.normals);
  cb.bindVertexBuffers(1, bufInfo.buffer, bufInfo.offset);
  bufInfo = resources.get(passIn.uvs);
  cb.bindVertexBuffers(2, bufInfo.buffer, bufInfo.offset);
  bufInfo = resources.get(passIn.indices);
  cb.bindIndexBuffer(bufInfo.buffer, bufInfo.offset, vk::IndexType::eUint32);

  cb.setLineWidth(lineWidth_);

  auto draw = [&](DrawGroup drawGroup) {
    auto drawGroupIdx = value(drawGroup);
    auto drawInfo = drawInfos.drawInfo[0][drawGroupIdx];
    if(drawInfo.maxCount == 0) return;
    cb.drawIndexedIndirectCount(
      drawInfo.drawCMD.buffer, drawInfo.drawCMD.offset, drawInfo.drawCMDCount.buffer,
      drawInfo.drawCMDCount.offset, drawInfo.maxCount, drawInfo.stride);
  };

  dev.begin(cb, "Subpass gbuffer brdf triangles");
  if(wireframe_) cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *gbWireFramePipe);
  else
    cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *gbTriPipe);
  draw(DrawGroup::BRDF);
  draw(DrawGroup::Reflective);
  draw(DrawGroup::Refractive);
  dev.end(cb);

  cb.nextSubpass(vk::SubpassContents::eInline);

  dev.begin(cb, "Subpass deferred lighting");
  vk::Pipeline pipe;
  if(atmosSetting.isEnabled() && shadowMapSetting.isEnabled()) pipe = *litAtmosCSMPipe;
  else if(atmosSetting.isEnabled())
    pipe = *litAtmosPipe;
  else if(shadowMapSetting.isEnabled())
    pipe = *litCSMPipe;
  else
    pipe = *litPipe;
  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipe);
  cb.draw(3, 1, 0, 0);
  dev.end(cb);

  cb.nextSubpass(vk::SubpassContents::eInline);

  dev.begin(cb, "Subpass unlit triangles");
  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *unlitTriPipe);
  draw(DrawGroup::Unlit);
  dev.end(cb);

  dev.begin(cb, "Subpass opaque lines");
  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *unlitLinePipe);
  draw(DrawGroup::OpaqueLines);
  dev.end(cb);

  cb.nextSubpass(vk::SubpassContents::eInline);

  dev.begin(cb, "Subpass transparent triangles");
  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *transTriPipe);
  draw(DrawGroup::Transparent);
  dev.end(cb);

  dev.begin(cb, "Subpass transparent lines");
  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *transLinePipe);
  draw(DrawGroup::TransparentLines);
  dev.end(cb);

  cb.endRenderPass();
}
}