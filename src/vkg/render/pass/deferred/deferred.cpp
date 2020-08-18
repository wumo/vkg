#include "deferred.hpp"

#include "vkg/math/frustum.hpp"

namespace vkg {

struct CamFrustumPassIn {
  FrameGraphResource<Camera *> camera;
};
struct CamFrustumPassOut {
  FrameGraphResource<std::span<Frustum>> camFrustum;
  FrameGraphResource<BufferInfo> camBuffer;
};

class CamFrustumPass: public Pass<CamFrustumPassIn, CamFrustumPassOut> {
public:
  auto setup(PassBuilder &builder, const CamFrustumPassIn &inputs)
    -> CamFrustumPassOut override {
    passIn = inputs;
    builder.read(passIn.camera);
    passOut = {
      .camFrustum = builder.create<std::span<Frustum>>("camFrustum"),
      .camBuffer = builder.create<BufferInfo>("camBuffer"),
    };

    frustums.resize(1);
    return passOut;
  }
  void compile(RenderContext &ctx, Resources &resources) override {
    if(!init) {
      init = true;
      camBuffer = buffer::devStorageBuffer(
        resources.device, sizeof(Camera::Desc) * ctx.numFrames, "camBuffer");
      resources.set(passOut.camBuffer, camBuffer->bufferInfo());
    }

    auto *camera = resources.get(passIn.camera);
    Frustum frustum{camera->proj() * camera->view()};

    frustums[0] = frustum;
    resources.set(passOut.camFrustum, {frustums});
  }

  void execute(RenderContext &ctx, Resources &resources) override {
    auto bufInfo = camBuffer->bufferInfo();
    auto *camera = resources.get(passIn.camera);
    auto desc = camera->desc();
    desc.frame = ctx.frameIndex;
    auto cb = ctx.graphics;
    ctx.device.begin(cb, "update camera");
    cb.updateBuffer(
      bufInfo.buffer, bufInfo.offset + sizeof(Camera::Desc) * ctx.frameIndex,
      sizeof(desc), &desc);
    cb.pipelineBarrier(
      vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eAllCommands, {},
      vk::MemoryBarrier{
        vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead},
      nullptr, nullptr);
    ctx.device.end(cb);
  }

  std::vector<Frustum> frustums;
  std::unique_ptr<Buffer> camBuffer;
  bool init{false};
};

auto DeferredPass::setup(PassBuilder &builder, const DeferredPassIn &inputs)
  -> DeferredPassOut {
  passIn = inputs;

  auto &cam = builder.newPass<CamFrustumPass>("CamFrustum", {passIn.camera});
  camBuffer = cam.out().camBuffer;

  auto &cull = builder.newPass<ComputeCullDrawCMD>(
    "Cull",
    {cam.out().camFrustum, passIn.meshInstances, passIn.meshInstancesCount,
     passIn.sceneConfig, passIn.primitives, passIn.matrices, passIn.drawGroupCount});
  cullPassOut = cull.out();

  builder.read(passIn.backImgVersion);
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
  passOut.backImgs = builder.write(passIn.backImgs);

  return passOut;
}
void DeferredPass::compile(RenderContext &ctx, Resources &resources) {
  auto backImgs = resources.get(passIn.backImgs);
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
    deferredPipeDef.scene(sceneSetDef);
    deferredPipeDef.gbuffer(gbufferSetDef);
    deferredPipeDef.atmosphere(atmosphereSetDef);
    deferredPipeDef.shadowMap(csmSetDef);
    deferredPipeDef.init(resources.device);

    descriptorPool = DescriptorPoolMaker()
                       .pipelineLayout(deferredPipeDef, ctx.numFrames)
                       .createUnique(resources.device);

    sceneSet = sceneSetDef.createSet(*descriptorPool);
    gbSets = gbufferSetDef.createSets(*descriptorPool, ctx.numFrames);
    shadowMapSet = csmSetDef.createSet(*descriptorPool);
    atmosphereSet = atmosphereSetDef.createSet(*descriptorPool);

    sceneSetDef.cameras(resources.get(camBuffer));
    sceneSetDef.meshInstances(resources.get(passIn.meshInstances));
    sceneSetDef.primitives(resources.get(passIn.primitives));
    sceneSetDef.matrices(resources.get(passIn.matrices));
    sceneSetDef.materials(resources.get(passIn.materials));

    lastNumValidSampler = numValidSampler;
    sceneSetDef.textures(0, uint32_t(samplers.size()), samplers.data());
    sceneSetDef.lighting(resources.get(passIn.lighting));
    sceneSetDef.lights(resources.get(passIn.lights));
    sceneSetDef.update(sceneSet);

    createRenderPass(resources.device, backImgs[0]->format());
    createGbufferPass(resources.device, sceneConfig);
    createLightingPass(resources.device, sceneConfig);
    createUnlitPass(resources.device, sceneConfig);
    createTransparentPass(resources.device, sceneConfig);
  }

  if(auto backImgVersion = resources.get(passIn.backImgVersion);
     backImgVersion > lastBackImgVersion) {
    lastBackImgVersion = backImgVersion;
    backImgs_ = backImgs;
    createAttachments(resources.device);
  }
  if(numValidSampler > lastNumValidSampler) {
    sceneSetDef.textures(
      lastNumValidSampler, numValidSampler - lastNumValidSampler,
      samplers.data() + lastNumValidSampler);
    sceneSetDef.update(sceneSet);
    lastNumValidSampler = numValidSampler;
  }

  if(auto atmosSetting = resources.get(passIn.atmosSetting); atmosSetting.isEnabled()) {
    if(auto version = resources.get(passIn.atmosphere.version);
       version > lastAtmosVersion) {
      lastAtmosVersion = version;
      atmosphereSetDef.atmosphere(resources.get(passIn.atmosphere.atmosphere));
      atmosphereSetDef.sun(resources.get(passIn.atmosphere.sun));
      atmosphereSetDef.transmittance(*resources.get(passIn.atmosphere.transmittance));
      atmosphereSetDef.scattering(*resources.get(passIn.atmosphere.scattering));
      atmosphereSetDef.irradiance(*resources.get(passIn.atmosphere.irradiance));
      atmosphereSetDef.update(atmosphereSet);
    }
  }

  if(auto shadowMapSetting = resources.get(passIn.shadowMapSetting);
     shadowMapSetting.isEnabled()) {
    csmSetDef.setting(resources.get(passIn.shadowmap.setting));
    csmSetDef.cascades(resources.get(passIn.shadowmap.cascades));
    csmSetDef.shadowMaps(*resources.get(passIn.shadowmap.shadowMaps));
    csmSetDef.update(shadowMapSet);
  }
}

auto DeferredPass::createAttachments(Device &device) -> void {
  auto nFrames = uint32_t(backImgs_.size());
  depthAtts.resize(nFrames);
  positionAtts.resize(nFrames);
  normalAtts.resize(nFrames);
  diffuseAtts.resize(nFrames);
  specularAtts.resize(nFrames);
  emissiveAtts.resize(nFrames);
  framebuffers.resize(nFrames);
  for(int i = 0; i < nFrames; ++i) {
    auto w = backImgs_[i]->extent().width;
    auto h = backImgs_[i]->extent().height;
    using vkUsage = vk::ImageUsageFlagBits;
    positionAtts[i] = image::make2DTex(
      toString("positionAtt_", i), device, w, h,
      vkUsage::eColorAttachment | vkUsage::eInputAttachment,
      vk::Format::eR32G32B32A32Sfloat);
    normalAtts[i] = image::make2DTex(
      toString("normalAtt", i), device, w, h,
      vkUsage::eColorAttachment | vkUsage::eInputAttachment,
      vk::Format::eR32G32B32A32Sfloat);
    diffuseAtts[i] = image::make2DTex(
      toString("diffuseAtt", i), device, w, h,
      vkUsage::eColorAttachment | vkUsage::eInputAttachment, vk::Format::eR8G8B8A8Unorm);
    specularAtts[i] = image::make2DTex(
      toString("specularAtt", i), device, w, h,
      vkUsage::eColorAttachment | vkUsage::eInputAttachment, vk::Format::eR8G8B8A8Unorm);
    emissiveAtts[i] = image::make2DTex(
      toString("emissiveAtt", i), device, w, h,
      vkUsage::eColorAttachment | vkUsage::eInputAttachment, vk::Format::eR8G8B8A8Unorm);
    depthAtts[i] = image::make2DTex(
      toString("depthAtt", i), device, w, h,
      vkUsage::eDepthStencilAttachment | vkUsage::eInputAttachment,
      vk::Format::eD32Sfloat, vk::SampleCountFlagBits::e1,
      vk::ImageAspectFlagBits::eDepth);

    gbufferSetDef.position(positionAtts[i]->imageView());
    gbufferSetDef.normal(normalAtts[i]->imageView());
    gbufferSetDef.diffuse(diffuseAtts[i]->imageView());
    gbufferSetDef.specular(specularAtts[i]->imageView());
    gbufferSetDef.emissive(emissiveAtts[i]->imageView());
    gbufferSetDef.depth(depthAtts[i]->imageView());
    gbufferSetDef.update(gbSets[i]);

    std::vector<vk::ImageView> attachments = {
      backImgs_[i]->imageView(),    positionAtts[i]->imageView(),
      normalAtts[i]->imageView(),   diffuseAtts[i]->imageView(),
      specularAtts[i]->imageView(), emissiveAtts[i]->imageView(),
      depthAtts[i]->imageView()};

    vk::FramebufferCreateInfo info{
      {}, *renderPass, uint32_t(attachments.size()), attachments.data(), w, h, 1};

    framebuffers[i] = device.vkDevice().createFramebufferUnique(info);
  }
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
  auto position = maker.attachmentCopy(backImg)
                    .format(vk::Format::eR32G32B32A32Sfloat)
                    .loadOp(vk::AttachmentLoadOp::eClear)
                    .storeOp(vk::AttachmentStoreOp::eDontCare)
                    .initialLayout(vk::ImageLayout::eUndefined)
                    .finalLayout(vk::ImageLayout::eColorAttachmentOptimal)
                    .index();
  auto normal =
    maker.attachmentCopy(position).format(vk::Format::eR32G32B32A32Sfloat).index();
  auto diffuse =
    maker.attachmentCopy(position).format(vk::Format::eR8G8B8A8Unorm).index();
  auto specular =
    maker.attachmentCopy(position).format(vk::Format::eR8G8B8A8Unorm).index();
  auto emissive =
    maker.attachmentCopy(position).format(vk::Format::eR8G8B8A8Unorm).index();
  auto depth = maker.attachmentCopy(position)
                 .format(vk::Format::eD32Sfloat)
                 .finalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                 .index();
  gbPass = maker.subpass(vk::PipelineBindPoint::eGraphics)
             .color(position)
             .color(normal)
             .color(diffuse)
             .color(specular)
             .color(emissive)
             .depthStencil(depth)
             .index();
  litPass = maker.subpass(vk::PipelineBindPoint::eGraphics)
              .color(backImg)
              .input(position)
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
