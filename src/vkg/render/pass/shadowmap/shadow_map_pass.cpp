#include "shadow_map_pass.hpp"
namespace vkg {

struct CSMFrustumPassIn {};
struct CSMFrustumPassOut {
  FrameGraphResource<std::span<Frustum>> frustums;
};

class CSMFrustumPass: public Pass<CSMFrustumPassIn, CSMFrustumPassOut> {
public:
  auto setup(PassBuilder &builder, const CSMFrustumPassIn &inputs)
    -> CSMFrustumPassOut override {
    passOut = {
      .frustums = builder.create<std::span<Frustum>>("CSMFrustums"),
    };
    return passOut;
  }
  void compile(RenderContext &ctx, Resources &resources) override {
    BasePass::compile(ctx, resources);
  }
  void execute(RenderContext &ctx, Resources &resources) override {
    BasePass::execute(ctx, resources);
  }
};

auto ShadowMapPass::setup(PassBuilder &builder, const ShadowMapPassIn &inputs)
  -> ShadowMapPassOut {
  passIn = inputs;

  auto &frustum = builder.newPass<CSMFrustumPass>("CSMFrustumPass", CSMFrustumPassIn{});

  auto &cull = builder.newPass<ComputeCullDrawCMD>(
    "ShadowMapCull", ComputeCullDrawCMDPassIn{
                       .frustums = frustum.out().frustums,
                       .meshInstances = passIn.meshInstances,
                       .meshInstancesCount = passIn.meshInstancesCount,
                       .sceneConfig = passIn.sceneConfig,
                       .primitives = passIn.primitives,
                       .matrices = passIn.matrices,
                       .maxPerGroup = passIn.maxPerGroup});
  cull.setPassCondition([]() { return false; });
  cullPassOut = cull.out();
  builder.read(passIn.shadowMapSetting);
  passOut = {
    .setting = builder.create<vk::Buffer>("ShadowMapSetting"),
    .cascades = builder.create<vk::Buffer>("Cascades"),
    .shadowMaps = builder.create<Texture *>("ShadowMaps"),
  };
  return passOut;
}
void ShadowMapPass::compile(RenderContext &ctx, Resources &resources) {
  auto setting = resources.get(passIn.shadowMapSetting);

  if(!setting.isEnabled() || setting.numCascades() == 0) return;

  if(
    shadowMaps == nullptr || shadowMaps->extent().width != setting.textureSize() ||
    shadowMaps->extent().depth != setting.numCascades()) {
    shadowMapSetting = buffer::hostUniformBuffer(ctx.device, sizeof(UBOShadowMapSetting));
    cascades =
      buffer::hostUniformBuffer(ctx.device, sizeof(CascadeDesc) * setting.numCascades());
    createRenderPass(ctx.device, setting);
    createTextures(ctx.device, setting);
    createDescriptorSets(ctx.device, setting);
    createPipeline(ctx.device, setting);
  }
}

auto ShadowMapPass::createRenderPass(Device &device, ShadowMapSetting &setting) -> void {
  RenderPassMaker maker;

  std::vector<uint32_t> depths;

  depths.resize(setting.numCascades());
  subpasses.resize(setting.numCascades());
  for(int i = 0; i < setting.numCascades(); ++i) {
    depths[i] = maker.attachment(vk::Format::eD32Sfloat)
                  .samples(vk::SampleCountFlagBits::e1)
                  .loadOp(vk::AttachmentLoadOp::eClear)
                  .storeOp(vk::AttachmentStoreOp::eStore)
                  .stencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                  .stencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                  .initialLayout(vk::ImageLayout::eUndefined)
                  .finalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                  .index();
    subpasses[i] =
      maker.subpass(vk::PipelineBindPoint::eGraphics).depthStencil(depths[i]).index();
    maker.dependency(VK_SUBPASS_EXTERNAL, subpasses[i])
      .srcStage(vk::PipelineStageFlagBits::eFragmentShader)
      .dstStage(vk::PipelineStageFlagBits::eEarlyFragmentTests)
      .srcAccess(vk::AccessFlagBits::eShaderRead)
      .dstAccess(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
      .flags(vk::DependencyFlagBits::eByRegion);
    maker.dependency(subpasses[i], VK_SUBPASS_EXTERNAL)
      .srcStage(vk::PipelineStageFlagBits::eLateFragmentTests)
      .dstStage(vk::PipelineStageFlagBits::eFragmentShader)
      .srcAccess(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
      .dstAccess(vk::AccessFlagBits::eShaderRead)
      .flags(vk::DependencyFlagBits::eByRegion);
  }

  renderPass = maker.createUnique(device.vkDevice());
}

auto ShadowMapPass::createTextures(Device &device, ShadowMapSetting &setting) -> void {
  shadowMaps = image::make2DArrayTex(
    "shadowMaps", device, setting.textureSize(), setting.textureSize(),
    setting.numCascades(),
    vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
    vk::Format::eD32Sfloat, vk::SampleCountFlagBits::e1, vk::ImageAspectFlagBits::eDepth,
    false);
  shadowMaps->setSampler(
    {{}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear});

  shadowMapLayerViews.resize(setting.numCascades());
  std::vector<vk::ImageView> attachments;
  attachments.resize(setting.numCascades());
  for(int i = 0; i < setting.numCascades(); ++i) {
    shadowMapLayerViews[i] = shadowMaps->createLayerImageView(i);
    attachments[i] = *shadowMapLayerViews[i];
  }
  vk::FramebufferCreateInfo info{
    {},
    *renderPass,
    uint32_t(attachments.size()),
    attachments.data(),
    setting.textureSize(),
    setting.textureSize(),
    1};
  framebuffer = device.vkDevice().createFramebufferUnique(info);
}

auto ShadowMapPass::createDescriptorSets(Device &device, ShadowMapSetting &setting)
  -> void {
  calcSetDef.init(device);

  calcPipeDef.set(calcSetDef);
  calcPipeDef.init(device);

  descriptorPool =
    DescriptorPoolMaker().pipelineLayout(calcPipeDef).createUnique(device.vkDevice());

  calcSet = calcSetDef.createSet(*descriptorPool);

  //  auto &drawQueue = dev.drawQueue;
  //
  //  calcSetDef.cascades(cascades->buffer());
  //  calcSetDef.matrices(dev.matrices->buffer());
  //  calcSetDef.update(calcSet);
}

auto ShadowMapPass::createPipeline(Device &device, ShadowMapSetting &setting) -> void {
  GraphicsPipelineMaker maker(device.vkDevice());

  //  maker.layout(calcPipeDef.layout())
  //    .renderPass(*renderPass)
  //    .vertexInputAuto(
  //      {{.stride = sizeof(Vertex::Position),
  //        .attributes = {{vk::Format::eR32G32B32Sfloat}}},
  //       {.stride = sizeof(Vertex::Normal), .attributes = {{vk::Format::eR32G32B32Sfloat}}},
  //       {.stride = sizeof(Vertex::UV), .attributes = {{vk::Format::eR32G32Sfloat}}}})
  //    .inputAssembly(vk::PrimitiveTopology::eTriangleList)
  //    .polygonMode(vk::PolygonMode::eFill)
  //    .cullMode(vk::CullModeFlagBits::eBack)
  //    .frontFace(vk::FrontFace::eCounterClockwise)
  //    .depthTestEnable(true)
  //    .depthWriteEnable(true)
  //    .depthCompareOp(vk::CompareOp::eLessOrEqual)
  //    .viewport({})
  //    .scissor({})
  //    .dynamicState(vk::DynamicState::eViewport)
  //    .dynamicState(vk::DynamicState::eScissor);
  //
  //  maker.shader(
  //    vk::ShaderStageFlagBits::eVertex,
  //    Shader{res::deferred::csm::csm_vert_span, sceneConfig.numCascades});
  //
  //  pipes.resize(sceneConfig.numCascades);
  //  for(int i = 0; i < sceneConfig.numCascades; ++i) {
  //    maker.subpass(subpasses[i]);
  //    pipes[i] = maker.createUnique();
  //  }
}

void ShadowMapPass::execute(RenderContext &ctx, Resources &resources) {}
}