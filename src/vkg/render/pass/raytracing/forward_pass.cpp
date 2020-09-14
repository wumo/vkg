#include "forward_pass.hpp"
#include "common/quad_vert.hpp"
#include "raytracing/raster/copy_depth_frag.hpp"
#include "raytracing/raster/raster_vert.hpp"
#include "raytracing/raster/opaque_frag.hpp"
#include "raytracing/raster/transparent_frag.hpp"

namespace vkg {
void ForwardPass::setup(vkg::PassBuilder &builder) {
  cullPassOut = builder
                  .newPass<ComputeCullDrawCMD>(
                    "CullDrawCMD",
                    {
                      passIn.camFrustum.camFrustum,
                      passIn.meshInstances,
                      passIn.meshInstancesCount,
                      passIn.sceneConfig,
                      passIn.primitives,
                      passIn.matrices,
                      passIn.countPerDrawGroup,
                    },
                    std::set{
                      ShadeModel::Transparent,
                      ShadeModel::TransparentLines,
                      ShadeModel::OpaqueLines,
                    })
                  .out();

  builder.read(cullPassOut);
  builder.read(passIn.traceRays);
  builder.read(passIn.camFrustum.camBuffer);
  builder.read(passIn.sceneConfig);
  builder.read(passIn.meshInstances);
  builder.read(passIn.primitives);
  builder.read(passIn.positions);
  builder.read(passIn.normals);
  builder.read(passIn.uvs);
  builder.read(passIn.indices);
  builder.read(passIn.matrices);
  builder.read(passIn.materials);
  builder.read(passIn.samplers);
  builder.read(passIn.numValidSampler);
  passOut = {
    .hdrImg = builder.write(passIn.traceRays.backImg),
  };
}
void ForwardPass::compile(vkg::RenderContext &ctx, vkg::Resources &resources) {
  auto *backImg = resources.get(passIn.traceRays.backImg);
  auto samplers = resources.get(passIn.samplers);
  auto numValidSampler = resources.get(passIn.numValidSampler);
  if(!init) {
    init = true;

    auto sceneConfig = resources.get(passIn.sceneConfig);
    sceneSetDef.textures.descriptorCount() = sceneConfig.maxNumTextures;
    sceneSetDef.init(resources.device);
    pipeDef.scene(sceneSetDef);
    pipeDef.init(resources.device);

    {
      createRenderPass(resources.device, backImg->format());
      createCopyDepthPass(resources.device, sceneConfig);
      createOpaquePass(resources.device, sceneConfig);
      createTransparentPass(resources.device, sceneConfig);
    }

    {
      descriptorPool = DescriptorPoolMaker()
                         .pipelineLayout(pipeDef, ctx.numFrames)
                         .createUnique(resources.device);

      frames.resize(ctx.numFrames);
      for(int i = 0; i < ctx.numFrames; ++i) {
        frames[i].sceneSet = sceneSetDef.createSet(*descriptorPool);
        ctx.device.name(frames[i].sceneSet, name + toString("sceneSet", i));
        sceneSetDef.textures(0, uint32_t(samplers.size()), samplers.data());
        sceneSetDef.update(frames[i].sceneSet);
      }
    }
  }
  auto &frame = frames[ctx.frameIndex];

  if(backImg != frame.backImg) {
    frame.backImg = backImg;
    auto w = frame.backImg->extent().width;
    auto h = frame.backImg->extent().height;
    frame.depthAtt = image::make2DTex(
      toString("depthAtt", ctx.frameIndex), ctx.device, w, h,
      vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::Format::eD32Sfloat,
      vk::SampleCountFlagBits::e1, vk::ImageAspectFlagBits::eDepth);
    std::vector<vk::ImageView> attachments = {
      frame.backImg->imageView(), frame.depthAtt->imageView()};
    vk::FramebufferCreateInfo info{
      {}, *renderPass, uint32_t(attachments.size()), attachments.data(), w, h, 1};
    frame.framebuffer = ctx.device.vkDevice().createFramebufferUnique(info);
  }
  if(numValidSampler > frame.lastNumValidSampler) {
    sceneSetDef.textures(
      frame.lastNumValidSampler, numValidSampler - frame.lastNumValidSampler,
      samplers.data() + frame.lastNumValidSampler);
    frame.lastNumValidSampler = numValidSampler;
  }
  sceneSetDef.depth(*resources.get(passIn.traceRays.depthImg));
  sceneSetDef.camera(resources.get(passIn.camFrustum.camBuffer));
  sceneSetDef.meshInstances(resources.get(passIn.meshInstances));
  sceneSetDef.primitives(resources.get(passIn.primitives));
  sceneSetDef.matrices(resources.get(passIn.matrices));
  sceneSetDef.materials(resources.get(passIn.materials));
  sceneSetDef.update(frame.sceneSet);
}
void ForwardPass::createRenderPass(Device &device, vk::Format format) {
  RenderPassMaker maker;
  auto color = maker.attachment(format)
                 .samples(vk::SampleCountFlagBits::e1)
                 .loadOp(vk::AttachmentLoadOp::eLoad)
                 .storeOp(vk::AttachmentStoreOp::eStore)
                 .stencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                 .stencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                 .initialLayout(vk::ImageLayout::eColorAttachmentOptimal)
                 .finalLayout(vk::ImageLayout::eColorAttachmentOptimal)
                 .index();
  auto depth = maker.attachmentCopy(color)
                 .format(vk::Format::eD32Sfloat)
                 .loadOp(vk::AttachmentLoadOp::eDontCare)
                 .storeOp(vk::AttachmentStoreOp::eDontCare)
                 .initialLayout(vk::ImageLayout::eUndefined)
                 .finalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                 .index();
  copyDepthPass =
    maker.subpass(vk::PipelineBindPoint::eGraphics).depthStencil(depth).index();
  opaquePass = maker.subpass(vk::PipelineBindPoint::eGraphics)
                 .color(color)
                 .depthStencil(depth)
                 .index();
  transparentPass = maker.subpass(vk::PipelineBindPoint::eGraphics)
                      .color(color)
                      .depthStencil(depth)
                      .index();
  maker.dependency(VK_SUBPASS_EXTERNAL, copyDepthPass)
    .srcStage(vk::PipelineStageFlagBits::eBottomOfPipe)
    .dstStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .srcAccess(vk::AccessFlagBits::eMemoryRead)
    .dstAccess(vk::AccessFlagBits::eColorAttachmentWrite)
    .flags(vk::DependencyFlagBits::eByRegion);
  maker.dependency(copyDepthPass, opaquePass)
    .srcStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .dstStage(
      vk::PipelineStageFlagBits::eEarlyFragmentTests |
      vk::PipelineStageFlagBits::eLateFragmentTests)
    .srcAccess(vk::AccessFlagBits::eColorAttachmentWrite)
    .dstAccess(vk::AccessFlagBits::eDepthStencilAttachmentRead)
    .flags(vk::DependencyFlagBits::eByRegion);
  maker.dependency(opaquePass, transparentPass)
    .srcStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .dstStage(
      vk::PipelineStageFlagBits::eEarlyFragmentTests |
      vk::PipelineStageFlagBits::eLateFragmentTests)
    .srcAccess(vk::AccessFlagBits::eColorAttachmentWrite)
    .dstAccess(vk::AccessFlagBits::eDepthStencilAttachmentRead)
    .flags(vk::DependencyFlagBits::eByRegion);
  maker.dependency(transparentPass, VK_SUBPASS_EXTERNAL)
    .srcStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .dstStage(vk::PipelineStageFlagBits::eBottomOfPipe)
    .srcAccess(vk::AccessFlagBits::eColorAttachmentWrite)
    .dstAccess(vk::AccessFlagBits::eMemoryRead)
    .flags(vk::DependencyFlagBits::eByRegion);

  renderPass = maker.createUnique(device.vkDevice());
}
void ForwardPass::createCopyDepthPass(Device &device, SceneConfig &sceneConfig) {
  GraphicsPipelineMaker maker(device.vkDevice());

  maker.layout(pipeDef.layout())
    .renderPass(*renderPass)
    .subpass(copyDepthPass)
    .inputAssembly(vk::PrimitiveTopology::eTriangleList)
    .polygonMode(vk::PolygonMode::eFill)
    .cullMode(vk::CullModeFlagBits::eNone)
    .frontFace(vk::FrontFace::eClockwise)
    .depthTestEnable(true)
    .depthWriteEnable(true)
    .depthCompareOp(vk::CompareOp::eAlways)
    .viewport({})
    .scissor({})
    .dynamicState(vk::DynamicState::eViewport)
    .dynamicState(vk::DynamicState::eScissor);

  maker.shader(vk::ShaderStageFlagBits::eVertex, Shader{shader::common::quad_vert_span})
    .shader(
      vk::ShaderStageFlagBits::eFragment,
      Shader{
        shader::raytracing::raster::copy_depth_frag_span, sceneConfig.maxNumTextures});

  copyDepthPipe = maker.createUnique();
  device.name(*copyDepthPipe, "copy depth pipe");
}
void ForwardPass::createOpaquePass(Device &device, SceneConfig &sceneConfig) {
  GraphicsPipelineMaker maker(device.vkDevice());

  maker.layout(pipeDef.layout())
    .renderPass(*renderPass)
    .subpass(opaquePass)
    .vertexInputAuto(
      {{.stride = sizeof(Vertex::Position),
        .attributes = {{vk::Format::eR32G32B32Sfloat}}},
       {.stride = sizeof(Vertex::Normal), .attributes = {{vk::Format::eR32G32B32Sfloat}}},
       {.stride = sizeof(Vertex::UV), .attributes = {{vk::Format::eR32G32Sfloat}}}})
    .inputAssembly(vk::PrimitiveTopology::eLineList)
    .polygonMode(vk::PolygonMode::eFill)
    .cullMode(vk::CullModeFlagBits::eNone)
    .frontFace(vk::FrontFace::eCounterClockwise)
    .depthTestEnable(true)
    .depthWriteEnable(true)
    .depthCompareOp(vk::CompareOp::eLessOrEqual)
    .viewport({})
    .scissor({})
    .dynamicState(vk::DynamicState::eViewport)
    .dynamicState(vk::DynamicState::eScissor)
    .dynamicState(vk::DynamicState::eLineWidth);

  maker.blendColorAttachment(false);

  maker
    .shader(
      vk::ShaderStageFlagBits::eVertex,
      Shader{shader::raytracing::raster::raster_vert_span, sceneConfig.maxNumTextures})
    .shader(
      vk::ShaderStageFlagBits::eFragment,
      Shader{shader::raytracing::raster::opaque_frag_span, sceneConfig.maxNumTextures});
  opaqueLinesPipe = maker.createUnique();
  device.name(*opaqueLinesPipe, "opaqueLinesPipe");
}
void ForwardPass::createTransparentPass(Device &device, SceneConfig &sceneConfig) {
  GraphicsPipelineMaker maker(device.vkDevice());

  maker.layout(pipeDef.layout())
    .renderPass(*renderPass)
    .subpass(transparentPass)
    .vertexInputAuto(
      {{.stride = sizeof(Vertex::Position),
        .attributes = {{vk::Format::eR32G32B32Sfloat}}},
       {.stride = sizeof(Vertex::Normal), .attributes = {{vk::Format::eR32G32B32Sfloat}}},
       {.stride = sizeof(Vertex::UV), .attributes = {{vk::Format::eR32G32Sfloat}}}})
    .inputAssembly(vk::PrimitiveTopology::eTriangleList)
    .polygonMode(vk::PolygonMode::eFill)
    .cullMode(vk::CullModeFlagBits::eNone)
    .frontFace(vk::FrontFace::eCounterClockwise)
    .depthTestEnable(true)
    .depthWriteEnable(false)
    .depthCompareOp(vk::CompareOp::eLessOrEqual)
    .viewport({})
    .scissor({})
    .dynamicState(vk::DynamicState::eViewport)
    .dynamicState(vk::DynamicState::eScissor);

  maker.blendColorAttachment(true)
    .srcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
    .dstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
    .colorBlendOp(vk::BlendOp::eAdd)
    .srcAlphaBlendFactor(vk::BlendFactor::eOne)
    .dstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
    .alphaBlendOp(vk::BlendOp::eAdd)
    .colorWriteMask(
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

  maker
    .shader(
      vk::ShaderStageFlagBits::eVertex,
      Shader{shader::raytracing::raster::raster_vert_span, sceneConfig.maxNumTextures})
    .shader(
      vk::ShaderStageFlagBits::eFragment,
      Shader{
        shader::raytracing::raster::transparent_frag_span, sceneConfig.maxNumTextures});
  transparentPipe = maker.createUnique();
  device.name(*transparentPipe, "transparentPipe");

  maker.inputAssembly(vk::PrimitiveTopology::eLineList)
    .dynamicState(vk::DynamicState::eLineWidth);
  transparentLinesPipe = maker.createUnique();
  device.name(*transparentLinesPipe, "transparentLinesPipe");
}
void ForwardPass::execute(vkg::RenderContext &ctx, vkg::Resources &resources) {
  auto drawInfos = resources.get(cullPassOut.drawCMDs);
  auto &frame = frames[ctx.frameIndex];

  auto cb = ctx.cb;
  image::transitTo(
    cb, *resources.get(passIn.traceRays.backImg),
    vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eColorAttachmentWrite,
    vk::PipelineStageFlagBits::eColorAttachmentOutput);
  image::transitTo(
    cb, *resources.get(passIn.traceRays.depthImg),
    vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderRead,
    vk::PipelineStageFlagBits::eFragmentShader);

  std::array<vk::ClearValue, 6> clearValues{
    vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 0.0f}},
    vk::ClearDepthStencilValue{1.0f, 0},
  };

  vk::RenderPassBeginInfo renderPassBeginInfo{
    *renderPass, *frame.framebuffer,
    vk::Rect2D{{0, 0}, {frame.backImg->extent().width, frame.backImg->extent().height}},
    uint32_t(clearValues.size()), clearValues.data()};
  cb.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
  vk::Viewport viewport{0,
                        0,
                        float(frame.backImg->extent().width),
                        float(frame.backImg->extent().height),
                        0.0f,
                        1.0f};
  cb.setViewport(0, viewport);
  vk::Rect2D scissor{
    {0, 0}, {frame.backImg->extent().width, frame.backImg->extent().height}};
  cb.setScissor(0, scissor);

  auto &dev = resources.device;
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics, pipeDef.layout(), pipeDef.scene.set(),
    frame.sceneSet, nullptr);

  auto bufInfo = resources.get(passIn.positions);
  cb.bindVertexBuffers(0, bufInfo.buffer, bufInfo.offset);
  bufInfo = resources.get(passIn.normals);
  cb.bindVertexBuffers(1, bufInfo.buffer, bufInfo.offset);
  bufInfo = resources.get(passIn.uvs);
  cb.bindVertexBuffers(2, bufInfo.buffer, bufInfo.offset);
  bufInfo = resources.get(passIn.indices);
  cb.bindIndexBuffer(bufInfo.buffer, bufInfo.offset, vk::IndexType::eUint32);

  pushConstant.frame = ctx.frameIndex;
  cb.pushConstants<PushConstant>(
    pipeDef.layout(), vk::ShaderStageFlagBits::eVertex, 0, pushConstant);

  auto draw = [&](ShadeModel shadeModel) {
    auto shadeModelIdx = value(shadeModel);
    auto drawInfo = drawInfos.cmdsPerShadeModel[0][shadeModelIdx];
    if(drawInfo.maxCount == 0) return;
    cb.drawIndexedIndirectCount(
      drawInfo.cmdBuf.buffer, drawInfo.cmdBuf.offset, drawInfo.countBuf.buffer,
      drawInfo.countBuf.offset, drawInfo.maxCount, drawInfo.stride);
  };

  dev.begin(cb, "Subpass copy depth");
  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *copyDepthPipe);
  cb.draw(3, 1, 0, 0);
  dev.end(cb);

  cb.nextSubpass(vk::SubpassContents::eInline);

  dev.begin(cb, "Subpass opaque lines");
  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *opaqueLinesPipe);
  cb.setLineWidth(lineWidth_);
  draw(ShadeModel::OpaqueLines);
  dev.end(cb);

  cb.nextSubpass(vk::SubpassContents::eInline);

  dev.begin(cb, "Subpass transparent triangles");
  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *transparentPipe);
  draw(ShadeModel::Transparent);
  dev.end(cb);

  dev.begin(cb, "Subpass transparent lines");
  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *transparentLinesPipe);
  cb.setLineWidth(lineWidth_);
  draw(ShadeModel::TransparentLines);
  dev.end(cb);

  cb.endRenderPass();
}

}