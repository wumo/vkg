#include "shadow_map_pass.hpp"
#include "vkg/render/model/aabb.hpp"
namespace vkg {

struct CSMFrustumPassIn {
  FrameGraphResource<AtmosphereSetting> atmosSetting;
  FrameGraphResource<ShadowMapSetting> shadowMapSetting;
  FrameGraphResource<Camera *> camera;
};
struct CSMFrustumPassOut {
  FrameGraphResource<std::span<Frustum>> frustums;
  FrameGraphResource<BufferInfo> cascades;
};

class CSMFrustumPass: public Pass<CSMFrustumPassIn, CSMFrustumPassOut> {
  struct CascadeDesc {
    glm::mat4 lightViewProj;
    glm::vec3 lightDir;
    float z;
  };

public:
  auto setup(PassBuilder &builder, const CSMFrustumPassIn &inputs)
    -> CSMFrustumPassOut override {
    passIn = inputs;
    builder.read(passIn);
    passOut = {
      .frustums = builder.create<std::span<Frustum>>("CSMFrustums"),
      .cascades = builder.create<BufferInfo>("cascades")};
    return passOut;
  }
  void compile(RenderContext &ctx, Resources &resources) override {
    auto atmos = resources.get(passIn.atmosSetting);
    auto setting = resources.get(passIn.shadowMapSetting);
    auto *camera = resources.get(passIn.camera);
    auto sunDir = atmos.sunDirection();

    const auto numCascades = setting.numCascades();
    frustums.resize(numCascades);

    auto zNear = camera->zNear();
    auto zFar = setting.zFar() == 0 ?
                  camera->zFar() :
                  glm::clamp(setting.zFar(), camera->zNear(), camera->zFar());
    auto p = camera->location();
    auto a = camera->width() / float(camera->height());
    auto tf = glm::tan(camera->fov() / 2);
    auto v = glm::normalize(camera->direction());
    auto r = normalize(cross(v, camera->worldUp()));
    auto u = glm::normalize(glm::cross(r, v));

    auto lambda = 0.5f;
    for(int i = 0; i < numCascades; ++i) {
      auto n = lambda * zNear * glm::pow(zFar / zNear, float(i) / numCascades) +
               (1 - lambda) * (zNear + (zFar - zNear) * (float(i) / numCascades));
      auto f = lambda * zNear * glm::pow(zFar / zNear, float(i + 1) / numCascades) +
               (1 - lambda) * (zNear + (zFar - zNear) * (float(i + 1) / numCascades));

      auto nearV = v * n;
      auto nearR = n * tf * a * r;
      auto nearU = n * tf * u;

      auto farV = v * f;
      auto farR = f * tf * a * r;
      auto farU = f * tf * u;

      glm::vec3 center{0};
      auto cam0 = p + nearV - nearR + nearU;
      center += cam0 / 8.f;
      auto cam1 = p + nearV - nearR - nearU;
      center += cam1 / 8.f;
      auto cam2 = p + nearV + nearR - nearU;
      center += cam2 / 8.f;
      auto cam3 = p + nearV + nearR + nearU;
      center += cam3 / 8.f;

      auto cam4 = p + farV - farR + farU;
      center += cam4 / 8.f;
      auto cam5 = p + farV - farR - farU;
      center += cam5 / 8.f;
      auto cam6 = p + farV + farR - farU;
      center += cam6 / 8.f;
      auto cam7 = p + farV + farR + farU;
      center += cam7 / 8.f;

      auto lUp = glm::vec3{0, 1, 0};
      if(glm::all(glm::epsilonEqual(
           cross(sunDir, lUp), glm::vec3{}, std::numeric_limits<float>::epsilon())))
        lUp = {0, 0, 1};

      auto lightView = viewMatrix(center, center + sunDir, lUp);
      auto lightViewInv = glm::inverse(lightView);

      AABB aabb;
      aabb.merge(lightView * glm::vec4(cam0, 1));
      aabb.merge(lightView * glm::vec4(cam1, 1));
      aabb.merge(lightView * glm::vec4(cam2, 1));
      aabb.merge(lightView * glm::vec4(cam3, 1));
      aabb.merge(lightView * glm::vec4(cam4, 1));
      aabb.merge(lightView * glm::vec4(cam5, 1));
      aabb.merge(lightView * glm::vec4(cam6, 1));
      aabb.merge(lightView * glm::vec4(cam7, 1));

      center = lightViewInv * glm::vec4(aabb.center(), 1);
      auto range = aabb.range();
      lightView = viewMatrix(center - sunDir * zFar / 2.f, center, lUp);

      auto lightProj =
        orthoMatrix(-range.x / 2, range.x / 2, -range.y / 2, range.y / 2, 0, zFar);

      auto lightViewProj = lightProj * lightView;
      cascades->ptr<CascadeDesc>()[i] = {lightViewProj, sunDir, f};
      frustums[i] = Frustum{lightViewProj};
    }
    resources.set(passOut.cascades, cascades->bufferInfo());
  }
  void execute(RenderContext &ctx, Resources &resources) override {}

  std::vector<Frustum> frustums;
  std::unique_ptr<Buffer> cascades;
};

auto ShadowMapPass::setup(PassBuilder &builder, const ShadowMapPassIn &inputs)
  -> ShadowMapPassOut {
  passIn = inputs;

  auto &frustum = builder.newPass<CSMFrustumPass>(
    "CSMFrustumPass", {passIn.atmosSetting, passIn.shadowMapSetting, passIn.camera});

  auto &cull = builder.newPass<ComputeCullDrawCMD>(
    "ShadowMapCull", {.frustums = frustum.out().frustums,
                      .meshInstances = passIn.meshInstances,
                      .meshInstancesCount = passIn.meshInstancesCount,
                      .sceneConfig = passIn.sceneConfig,
                      .primitives = passIn.primitives,
                      .matrices = passIn.matrices,
                      .transformStride = passIn.transformStride,
                      .maxPerGroup = passIn.maxPerGroup});
  cull.enableIf([]() { return false; });
  cullPassOut = cull.out();
  builder.read(passIn.shadowMapSetting);
  passOut = {
    .setting = builder.create<BufferInfo>("ShadowMapSetting"),
    .cascades = frustum.out().cascades,
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
    createRenderPass(ctx.device, setting);
    createTextures(ctx.device, setting);
    createDescriptorSets(ctx.device, setting);
    createPipeline(ctx.device, setting);

    resources.set(passOut.setting, shadowMapSetting->bufferInfo());
    resources.set(passOut.shadowMaps, shadowMaps.get());
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