#include "deferred.hpp"

namespace vkg {
void DeferredPass::execute(RenderContext &ctx, Resources &resources) {
    auto drawInfos = resources.get(passIn.cullCMD.drawCMDs);
    auto atmosSetting = resources.get(passIn.atmosSetting);
    auto shadowMapSetting = resources.get(passIn.shadowMapSetting);

    auto &frame = frames[ctx.frameIndex];

    auto cb = ctx.cb;
    image::transitTo(
        cb, *frame.backImg, vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eColorAttachmentWrite,
        vk::PipelineStageFlagBits::eColorAttachmentOutput);
    image::transitTo(
        cb, *frame.transColorAtt, vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eColorAttachmentWrite,
        vk::PipelineStageFlagBits::eColorAttachmentOutput);
    image::transitTo(
        cb, *frame.revealAtt, vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eColorAttachmentWrite,
        vk::PipelineStageFlagBits::eColorAttachmentOutput);

    std::array<vk::ClearValue, 8> clearValues{
        vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 0.0f}},
        vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 0.0f}},
        vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 0.0f}},
        vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 0.0f}},
        vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 0.0f}},
        vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 0.0f}},
        vk::ClearColorValue{std::array{1.0f, 0.0f, 0.0f, 0.0f}},
        vk::ClearDepthStencilValue{1.0f, 0},
    };

    vk::RenderPassBeginInfo renderPassBeginInfo{
        *renderPass, *frame.framebuffer,
        vk::Rect2D{{0, 0}, {frame.backImg->extent().width, frame.backImg->extent().height}},
        uint32_t(clearValues.size()), clearValues.data()};
    cb.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    vk::Viewport viewport{0,    0,   float(frame.backImg->extent().width), float(frame.backImg->extent().height),
                          0.0f, 1.0f};
    cb.setViewport(0, viewport);
    vk::Rect2D scissor{{0, 0}, {frame.backImg->extent().width, frame.backImg->extent().height}};
    cb.setScissor(0, scissor);

    auto &dev = resources.device;
    cb.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, pipeDef.layout(), pipeDef.scene.set(), frame.sceneSet, nullptr);
    cb.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, pipeDef.layout(), pipeDef.gbuffer.set(), frame.gbSet, nullptr);
    cb.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, pipeDef.layout(), pipeDef.trans.set(), frame.transSet, nullptr);
    if(atmosSetting.isEnabled())
        cb.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, pipeDef.layout(), pipeDef.atmosphere.set(), frame.atmosphereSet, nullptr);
    if(shadowMapSetting.isEnabled())
        cb.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, pipeDef.layout(), pipeDef.shadowMap.set(), frame.shadowMapSet, nullptr);

    auto bufInfo = resources.get(passIn.positions);
    cb.bindVertexBuffers(0, bufInfo.buffer, bufInfo.offset);
    bufInfo = resources.get(passIn.normals);
    cb.bindVertexBuffers(1, bufInfo.buffer, bufInfo.offset);
    bufInfo = resources.get(passIn.uvs);
    cb.bindVertexBuffers(2, bufInfo.buffer, bufInfo.offset);
    bufInfo = resources.get(passIn.indices);
    cb.bindIndexBuffer(bufInfo.buffer, bufInfo.offset, vk::IndexType::eUint32);

    pushConstant.frame = ctx.frameIndex;
    cb.pushConstants<PushConstant>(pipeDef.layout(), vk::ShaderStageFlagBits::eVertex, 0, pushConstant);

    auto draw = [&](ShadeModel shadeModel) {
        auto shadeModelIdx = value(shadeModel);
        auto drawInfo = drawInfos.cmdsPerShadeModel[0][shadeModelIdx];
        if(drawInfo.maxCount == 0) return;
        cb.drawIndexedIndirectCount(
            drawInfo.cmdBuf.buffer, drawInfo.cmdBuf.offset, drawInfo.countBuf.buffer, drawInfo.countBuf.offset,
            drawInfo.maxCount, drawInfo.stride);
    };

    cb.setLineWidth(lineWidth_);
    dev.begin(cb, "Subpass gbuffer brdf triangles");
    if(wireframe_) cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *gbWireFramePipe);
    else
        cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *gbTriPipe);
    draw(ShadeModel::BRDF);
    draw(ShadeModel::Reflective);
    draw(ShadeModel::Refractive);
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
    draw(ShadeModel::Unlit);
    dev.end(cb);

    dev.begin(cb, "Subpass opaque lines");
    cb.setLineWidth(lineWidth_);
    cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *unlitLinePipe);
    draw(ShadeModel::OpaqueLines);
    dev.end(cb);

    cb.nextSubpass(vk::SubpassContents::eInline);

    dev.begin(cb, "Subpass transparent triangles");
    cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *transTriPipe);
    draw(ShadeModel::Transparent);
    dev.end(cb);

    dev.begin(cb, "Subpass transparent lines");
    cb.setLineWidth(lineWidth_);
    cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *transLinePipe);
    draw(ShadeModel::TransparentLines);
    dev.end(cb);

    cb.nextSubpass(vk::SubpassContents::eInline);

    dev.begin(cb, "Subpass composite");
    cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *compositePipe);
    cb.draw(3, 1, 0, 0);
    dev.end(cb);

    cb.endRenderPass();
}
}