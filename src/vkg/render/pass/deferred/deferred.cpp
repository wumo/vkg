#include "deferred.hpp"

namespace vkg {

void DeferredPass::setup(PassBuilder &builder) {
  auto &cam = builder.newPass<CamFrustumPass>("CamFrustum", {passIn.camera});
  camBuffer = cam.out().camBuffer;

  auto &cull = builder.newPass<ComputeCullDrawCMD>(
    "Cull",
    {cam.out().camFrustum, passIn.meshInstances, passIn.meshInstancesCount,
     passIn.sceneConfig, passIn.primitives, passIn.matrices, passIn.drawGroupCount});
  cullPassOut = cull.out();

  builder.read(passIn.sceneConfig);
  builder.read(cam.out().camBuffer);
  builder.read(passIn.meshInstances);
  builder.read(passIn.primitives);
  builder.read(passIn.matrices);
  builder.read(passIn.materials);
  builder.read(passIn.samplers);
  builder.read(passIn.lighting);
  builder.read(passIn.lights);
  builder.read(passIn.drawGroupCount);
  builder.read(cull.out());
  builder.read(passIn.atmosSetting);
  builder.read(passIn.atmosphere);
  builder.read(passIn.shadowMapSetting);
  builder.read(passIn.shadowmap);
  passOut.backImg = builder.write(passIn.backImg);
}
void DeferredPass::compile(RenderContext &ctx, Resources &resources) {
  auto *backImg = resources.get(passIn.backImg);
  auto samplers = resources.get(passIn.samplers);
  auto numValidSampler = resources.get(passIn.numValidSampler);

  if(!init) {
    init = true;

    auto sceneConfig = resources.get(passIn.sceneConfig);
    sceneSetDef.textures.descriptorCount() = sceneConfig.maxNumTextures;
    sceneSetDef.init(resources.device);
    gbufferSetDef.init(resources.device);
    csmSetDef.init(resources.device);
    atmosphereSetDef.init(resources.device);
    pipeDef.scene(sceneSetDef);
    pipeDef.gbuffer(gbufferSetDef);
    pipeDef.atmosphere(atmosphereSetDef);
    pipeDef.shadowMap(csmSetDef);
    pipeDef.init(resources.device);

    createRenderPass(resources.device, backImg->format());
    createGbufferPass(resources.device, sceneConfig);
    createLightingPass(resources.device, sceneConfig);
    createUnlitPass(resources.device, sceneConfig);
    createTransparentPass(resources.device, sceneConfig);

    {
      descriptorPool = DescriptorPoolMaker()
                         .pipelineLayout(pipeDef, ctx.numFrames)
                         .createUnique(resources.device);

      frames.resize(ctx.numFrames);
      for(int i = 0; i < ctx.numFrames; ++i) {
        frames[i].sceneSet = sceneSetDef.createSet(*descriptorPool);
        frames[i].gbSet = gbufferSetDef.createSet(*descriptorPool);
        frames[i].shadowMapSet = csmSetDef.createSet(*descriptorPool);
        frames[i].atmosphereSet = atmosphereSetDef.createSet(*descriptorPool);
        sceneSetDef.textures(0, uint32_t(samplers.size()), samplers.data());
        sceneSetDef.update(frames[i].sceneSet);
      }
    }
  }

  auto &frame = frames[ctx.frameIndex];

  if(backImg != frame.backImg) {
    frame.backImg = backImg;
    createAttachments(resources.device, ctx.frameIndex);
  }
  if(numValidSampler > frame.lastNumValidSampler) {
    sceneSetDef.textures(
      frame.lastNumValidSampler, numValidSampler - frame.lastNumValidSampler,
      samplers.data() + frame.lastNumValidSampler);
    frame.lastNumValidSampler = numValidSampler;
  }
  sceneSetDef.camera(resources.get(camBuffer));
  sceneSetDef.meshInstances(resources.get(passIn.meshInstances));
  sceneSetDef.primitives(resources.get(passIn.primitives));
  sceneSetDef.matrices(resources.get(passIn.matrices));
  sceneSetDef.materials(resources.get(passIn.materials));
  sceneSetDef.lighting(resources.get(passIn.lighting));
  sceneSetDef.lights(resources.get(passIn.lights));
  sceneSetDef.update(frame.sceneSet);

  if(auto atmosSetting = resources.get(passIn.atmosSetting); atmosSetting.isEnabled()) {
    atmosphereSetDef.atmosphere(resources.get(passIn.atmosphere.atmosphere));
    atmosphereSetDef.sun(resources.get(passIn.atmosphere.sun));
    atmosphereSetDef.transmittance(*resources.get(passIn.atmosphere.transmittance));
    atmosphereSetDef.scattering(*resources.get(passIn.atmosphere.scattering));
    atmosphereSetDef.irradiance(*resources.get(passIn.atmosphere.irradiance));
    atmosphereSetDef.update(frame.atmosphereSet);
  }

  if(auto shadowMapSetting = resources.get(passIn.shadowMapSetting);
     shadowMapSetting.isEnabled()) {
    csmSetDef.setting(resources.get(passIn.shadowmap.settingBuffer));
    csmSetDef.cascades(resources.get(passIn.shadowmap.cascades));
    csmSetDef.shadowMaps(*resources.get(passIn.shadowmap.shadowMaps));
    csmSetDef.update(frame.shadowMapSet);
  }
}

auto DeferredPass::createAttachments(Device &device, uint32_t frameIdx) -> void {
  auto &frame = frames[frameIdx];
  auto w = frame.backImg->extent().width;
  auto h = frame.backImg->extent().height;
  using vkUsage = vk::ImageUsageFlagBits;
  frame.normalAtt = image::make2DTex(
    toString("normalAtt", frameIdx), device, w, h,
    vkUsage::eColorAttachment | vkUsage::eInputAttachment,
    vk::Format::eR16G16B16A16Sfloat);
  frame.diffuseAtt = image::make2DTex(
    toString("diffuseAtt", frameIdx), device, w, h,
    vkUsage::eColorAttachment | vkUsage::eInputAttachment, vk::Format::eR8G8B8A8Unorm);
  frame.specularAtt = image::make2DTex(
    toString("specularAtt", frameIdx), device, w, h,
    vkUsage::eColorAttachment | vkUsage::eInputAttachment, vk::Format::eR8G8B8A8Unorm);
  frame.emissiveAtt = image::make2DTex(
    toString("emissiveAtt", frameIdx), device, w, h,
    vkUsage::eColorAttachment | vkUsage::eInputAttachment, vk::Format::eR8G8B8A8Unorm);
  frame.depthAtt = image::make2DTex(
    toString("depthAtt", frameIdx), device, w, h,
    vkUsage::eDepthStencilAttachment | vkUsage::eInputAttachment, vk::Format::eD32Sfloat,
    vk::SampleCountFlagBits::e1, vk::ImageAspectFlagBits::eDepth);

  gbufferSetDef.normal(frame.normalAtt->imageView());
  gbufferSetDef.diffuse(frame.diffuseAtt->imageView());
  gbufferSetDef.specular(frame.specularAtt->imageView());
  gbufferSetDef.emissive(frame.emissiveAtt->imageView());
  gbufferSetDef.depth(frame.depthAtt->imageView());
  gbufferSetDef.update(frame.gbSet);

  std::vector<vk::ImageView> attachments = {
    frame.backImg->imageView(),     frame.normalAtt->imageView(),
    frame.diffuseAtt->imageView(),  frame.specularAtt->imageView(),
    frame.emissiveAtt->imageView(), frame.depthAtt->imageView()};

  vk::FramebufferCreateInfo info{
    {}, *renderPass, uint32_t(attachments.size()), attachments.data(), w, h, 1};

  frame.framebuffer = device.vkDevice().createFramebufferUnique(info);
}
auto DeferredPass::createRenderPass(Device &device, vk::Format format) -> void {
  RenderPassMaker maker;
  auto backImg = maker.attachment(format)
                   .samples(vk::SampleCountFlagBits::e1)
                   .loadOp(vk::AttachmentLoadOp::eClear)
                   .storeOp(vk::AttachmentStoreOp::eStore)
                   .stencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                   .stencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                   .initialLayout(vk::ImageLayout::eColorAttachmentOptimal)
                   .finalLayout(vk::ImageLayout::eColorAttachmentOptimal)
                   .index();
  auto normal = maker.attachmentCopy(backImg)
                  .format(vk::Format::eR16G16B16A16Sfloat)
                  .loadOp(vk::AttachmentLoadOp::eClear)
                  .storeOp(vk::AttachmentStoreOp::eDontCare)
                  .initialLayout(vk::ImageLayout::eUndefined)
                  .finalLayout(vk::ImageLayout::eColorAttachmentOptimal)
                  .index();
  auto diffuse = maker.attachmentCopy(normal).format(vk::Format::eR8G8B8A8Unorm).index();
  auto specular = maker.attachmentCopy(normal).format(vk::Format::eR8G8B8A8Unorm).index();
  auto emissive = maker.attachmentCopy(normal).format(vk::Format::eR8G8B8A8Unorm).index();
  auto depth = maker.attachmentCopy(normal)
                 .format(vk::Format::eD32Sfloat)
                 .finalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                 .index();
  gbPass = maker.subpass(vk::PipelineBindPoint::eGraphics)
             .color(normal)
             .color(diffuse)
             .color(specular)
             .color(emissive)
             .depthStencil(depth)
             .index();
  litPass = maker.subpass(vk::PipelineBindPoint::eGraphics)
              .color(backImg)
              .input(normal)
              .input(diffuse)
              .input(specular)
              .input(emissive)
              .input(depth)
              .index();
  unlitPass = maker.subpass(vk::PipelineBindPoint::eGraphics)
                .color(backImg)
                .depthStencil(depth)
                .index();
  transPass = maker.subpass(vk::PipelineBindPoint::eGraphics)
                .color(backImg)
                .depthStencil(depth)
                .index();
  maker.dependency(VK_SUBPASS_EXTERNAL, gbPass)
    .srcStage(vk::PipelineStageFlagBits::eBottomOfPipe)
    .dstStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .srcAccess(vk::AccessFlagBits::eMemoryRead)
    .dstAccess(vk::AccessFlagBits::eColorAttachmentWrite)
    .flags(vk::DependencyFlagBits::eByRegion);
  maker.dependency(gbPass, litPass)
    .srcStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .dstStage(vk::PipelineStageFlagBits::eFragmentShader)
    .srcAccess(vk::AccessFlagBits::eColorAttachmentWrite)
    .dstAccess(vk::AccessFlagBits::eInputAttachmentRead)
    .flags(vk::DependencyFlagBits::eByRegion);
  maker.dependency(litPass, unlitPass)
    .srcStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .dstStage(
      vk::PipelineStageFlagBits::eEarlyFragmentTests |
      vk::PipelineStageFlagBits::eLateFragmentTests)
    .srcAccess(vk::AccessFlagBits::eColorAttachmentWrite)
    .dstAccess(
      vk::AccessFlagBits::eDepthStencilAttachmentRead |
      vk::AccessFlagBits::eDepthStencilAttachmentWrite)
    .flags(vk::DependencyFlagBits::eByRegion);
  maker.dependency(unlitPass, transPass)
    .srcStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .dstStage(
      vk::PipelineStageFlagBits::eEarlyFragmentTests |
      vk::PipelineStageFlagBits::eLateFragmentTests)
    .srcAccess(vk::AccessFlagBits::eColorAttachmentWrite)
    .dstAccess(vk::AccessFlagBits::eDepthStencilAttachmentRead)
    .flags(vk::DependencyFlagBits::eByRegion);
  maker.dependency(transPass, VK_SUBPASS_EXTERNAL)
    .srcStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .dstStage(vk::PipelineStageFlagBits::eBottomOfPipe)
    .srcAccess(vk::AccessFlagBits::eColorAttachmentWrite)
    .dstAccess(vk::AccessFlagBits::eMemoryRead)
    .flags(vk::DependencyFlagBits::eByRegion);

  renderPass = maker.createUnique(device);
}

}
