#include "deferred.hpp"

#include "vkg/math/frustum.hpp"
#include "compute_cull_drawcmd.hpp"

namespace vkg {

struct CamFrustumPassIn {
  FrameGraphResource<Camera *> camera;
};
struct CamFrustumPassOut {
  FrameGraphResource<vk::Buffer> camFrustum;
};

class CamFrustumPass: public Pass<CamFrustumPassIn, CamFrustumPassOut> {
public:
  auto setup(PassBuilder &builder, const CamFrustumPassIn &inputs)
    -> CamFrustumPassOut override {
    passIn = inputs;
    builder.read(passIn.camera);
    passOut.camFrustum = builder.create<vk::Buffer>("camFrustum");

    return passOut;
  }
  void compile(Resources &resources) override {
    if(!init) {
      init = true;
      camFrustum =
        buffer::devUniformBuffer(resources.device, sizeof(Frustum), name + "_camFrustum");

      resources.set(passOut.camFrustum, camFrustum->buffer());
    }
  }
  void execute(RenderContext &ctx, Resources &resources) override {
    auto cb = ctx.compute;

    auto *camera = resources.get(passIn.camera);
    Frustum frustum{camera->proj() * camera->view()};
    cb.updateBuffer(camFrustum->buffer(), 0, sizeof(Frustum), &frustum);

    cb.pipelineBarrier(
      vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader, {},
      vk::MemoryBarrier{
        vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead},
      nullptr, nullptr);
  }

  CamFrustumPassIn passIn;
  CamFrustumPassOut passOut;
  bool init{false};

  std::unique_ptr<Buffer> camFrustum;
};

auto DeferredPass::setup(PassBuilder &builder, const DeferredPassIn &inputs)
  -> DeferredPassOut {
  passIn = inputs;

  builder.read(passIn.sceneConfig);
  builder.read(passIn.cameraBuffer);
  builder.read(passIn.meshInstances);
  builder.read(passIn.primitives);
  builder.read(passIn.matrices);
  builder.read(passIn.materials);
  builder.read(passIn.samplers);
  builder.read(passIn.lighting);
  builder.read(passIn.lights);
  passOut.backImg = builder.write(passIn.backImg);

  auto camPassOut =
    builder.newPass<CamFrustumPass>("CamFrustum", CamFrustumPassIn{passIn.camera});

  auto cullCMDOut = builder.newPass<ComputeCullDrawCMD>(
    "Cull",
    ComputeCullDrawCMDPassIn{
      camPassOut.camFrustum, passIn.meshInstances, passIn.meshInstancesCount,
      passIn.sceneConfig, passIn.primitives, passIn.matrices, passIn.drawGroupCount});

  builder.read(cullCMDOut.drawCMDBuffer);
  builder.read(cullCMDOut.drawCMDCountBuffer);

  return passOut;
}
void DeferredPass::compile(Resources &resources) {
  auto backImg = resources.get(passIn.backImg);
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
                       .pipelineLayout(deferredPipeDef)
                       .createUnique(resources.device);

    sceneSet = sceneSetDef.createSet(*descriptorPool);
    gbufferSet = gbufferSetDef.createSet(*descriptorPool);
    shadowMapSet = csmSetDef.createSet(*descriptorPool);
    atmosphereSet = atmosphereSetDef.createSet(*descriptorPool);

    sceneSetDef.cam(resources.get(passIn.cameraBuffer));
    sceneSetDef.meshInstances(resources.get(passIn.meshInstances));
    sceneSetDef.primitives(resources.get(passIn.primitives));
    sceneSetDef.matrices(resources.get(passIn.matrices));
    sceneSetDef.materials(resources.get(passIn.materials));

    lastNumValidSampler = numValidSampler;
    sceneSetDef.textures(0, uint32_t(samplers->size()), samplers->data());
    sceneSetDef.lighting(resources.get(passIn.lighting));
    sceneSetDef.lights(resources.get(passIn.lights));
    sceneSetDef.update(sceneSet);

    createRenderPass(resources.device, sceneConfig, backImg->format());
  }

  if(backImg != backImg_) {
    backImg_ = backImg;
    createAttachments(resources.device, *backImg);
  }
  if(numValidSampler > lastNumValidSampler) {
    sceneSetDef.textures(
      lastNumValidSampler, numValidSampler - lastNumValidSampler,
      samplers->data() + lastNumValidSampler);
    lastNumValidSampler = numValidSampler;
  }
}

auto DeferredPass::createAttachments(Device &device, Texture &backImg) -> void {
  auto w = backImg.extent().width;
  auto h = backImg.extent().height;
  using vkUsage = vk::ImageUsageFlagBits;
  colorAtt = image::make2DTex(
    "colorAtt", device, w, h, vkUsage::eColorAttachment | vkUsage::eTransferSrc,
    backImg.format(), sampleCount);
  positionAtt = image::make2DTex(
    "positionAtt", device, w, h, vkUsage::eColorAttachment | vkUsage::eInputAttachment,
    vk::Format::eR32G32B32A32Sfloat, sampleCount);
  normalAtt = image::make2DTex(
    "normalAtt", device, w, h, vkUsage::eColorAttachment | vkUsage::eInputAttachment,
    vk::Format::eR32G32B32A32Sfloat, sampleCount);
  diffuseAtt = image::make2DTex(
    "diffuseAtt", device, w, h, vkUsage::eColorAttachment | vkUsage::eInputAttachment,
    vk::Format::eR8G8B8A8Unorm, sampleCount);
  specularAtt = image::make2DTex(
    "specularAtt", device, w, h, vkUsage::eColorAttachment | vkUsage::eInputAttachment,
    vk::Format::eR8G8B8A8Unorm, sampleCount);
  emissiveAtt = image::make2DTex(
    "emissiveAtt", device, w, h, vkUsage::eColorAttachment | vkUsage::eInputAttachment,
    vk::Format::eR8G8B8A8Unorm, sampleCount);
  depthAtt = image::make2DTex(
    "depthAtt", device, w, h,
    vkUsage::eDepthStencilAttachment | vkUsage::eInputAttachment, vk::Format::eD32Sfloat,
    sampleCount, vk::ImageAspectFlagBits::eDepth);

  gbufferSetDef.position(positionAtt->imageView());
  gbufferSetDef.normal(normalAtt->imageView());
  gbufferSetDef.diffuse(diffuseAtt->imageView());
  gbufferSetDef.specular(specularAtt->imageView());
  gbufferSetDef.emissive(emissiveAtt->imageView());
  gbufferSetDef.depth(depthAtt->imageView());
  gbufferSetDef.update(gbufferSet);

  std::vector<vk::ImageView> attachments = {
    backImg.imageView(),      colorAtt->imageView(),   positionAtt->imageView(),
    normalAtt->imageView(),   diffuseAtt->imageView(), specularAtt->imageView(),
    emissiveAtt->imageView(), depthAtt->imageView()};

  vk::FramebufferCreateInfo info{
    {}, *renderPass, uint32_t(attachments.size()), attachments.data(), w, h, 1};
  framebuffer = device.vkDevice().createFramebufferUnique(info);
}
auto DeferredPass::createRenderPass(
  Device &device, SceneConfig sceneConfig, vk::Format format) -> void {
  sampleCount = static_cast<vk::SampleCountFlagBits>(sceneConfig.sampleCount);
  RenderPassMaker maker;
  auto backImg = maker.attachment(format)
                   .samples(vk::SampleCountFlagBits::e1)
                   .loadOp(vk::AttachmentLoadOp::eClear)
                   .storeOp(vk::AttachmentStoreOp::eStore)
                   .stencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                   .stencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                   .initialLayout(vk::ImageLayout::eUndefined)
                   .finalLayout(vk::ImageLayout::eTransferDstOptimal)
                   .index();
  auto color = maker.attachmentCopy(backImg)
                 .samples(sampleCount)
                 .storeOp(vk::AttachmentStoreOp::eStore)
                 .finalLayout(vk::ImageLayout::eTransferSrcOptimal)
                 .index();
  auto position = maker.attachmentCopy(color)
                    .format(vk::Format::eR32G32B32A32Sfloat)
                    .storeOp(vk::AttachmentStoreOp::eDontCare)
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
  color = sceneConfig.sampleCount == 1 ? backImg : color;
  gbufferPass = maker.subpass(vk::PipelineBindPoint::eGraphics)
                  .color(position)
                  .color(normal)
                  .color(diffuse)
                  .color(specular)
                  .color(emissive)
                  .depthStencil(depth)
                  .index();
  lightingPass = maker.subpass(vk::PipelineBindPoint::eGraphics)
                   .color(color)
                   .input(position)
                   .input(normal)
                   .input(diffuse)
                   .input(specular)
                   .input(emissive)
                   .input(depth)
                   .index();
  transPass = maker.subpass(vk::PipelineBindPoint::eGraphics)
                .color(color)
                .depthStencil(depth)
                .index();
  resolvePass =
    sceneConfig.sampleCount > 1 ?
      maker.subpass(vk::PipelineBindPoint::eGraphics).color(color, backImg).index() :
      maker.subpass(vk::PipelineBindPoint::eGraphics).color(color).index();
  maker.dependency(VK_SUBPASS_EXTERNAL, gbufferPass)
    .srcStage(vk::PipelineStageFlagBits::eBottomOfPipe)
    .dstStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .srcAccess(vk::AccessFlagBits::eMemoryRead)
    .dstAccess(vk::AccessFlagBits::eColorAttachmentWrite)
    .flags(vk::DependencyFlagBits::eByRegion);
  maker.dependency(gbufferPass, lightingPass)
    .srcStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .dstStage(vk::PipelineStageFlagBits::eFragmentShader)
    .srcAccess(vk::AccessFlagBits::eColorAttachmentWrite)
    .dstAccess(vk::AccessFlagBits::eInputAttachmentRead)
    .flags(vk::DependencyFlagBits::eByRegion);
  maker.dependency(lightingPass, transPass)
    .srcStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .dstStage(
      vk::PipelineStageFlagBits::eEarlyFragmentTests |
      vk::PipelineStageFlagBits::eLateFragmentTests)
    .srcAccess(vk::AccessFlagBits::eColorAttachmentWrite)
    .dstAccess(vk::AccessFlagBits::eDepthStencilAttachmentRead)
    .flags(vk::DependencyFlagBits::eByRegion);
  maker.dependency(transPass, resolvePass)
    .srcStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .dstStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .srcAccess(vk::AccessFlagBits::eColorAttachmentWrite)
    .dstAccess(vk::AccessFlagBits::eColorAttachmentRead)
    .flags(vk::DependencyFlagBits::eByRegion);
  maker.dependency(resolvePass, VK_SUBPASS_EXTERNAL)
    .srcStage(vk::PipelineStageFlagBits::eColorAttachmentOutput)
    .dstStage(vk::PipelineStageFlagBits::eBottomOfPipe)
    .srcAccess(vk::AccessFlagBits::eColorAttachmentWrite)
    .dstAccess(vk::AccessFlagBits::eMemoryRead)
    .flags(vk::DependencyFlagBits::eByRegion);

  renderPass = maker.createUnique(device);
}

}
