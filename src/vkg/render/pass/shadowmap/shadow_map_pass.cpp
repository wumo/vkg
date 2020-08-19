#include "shadow_map_pass.hpp"
#include "vkg/render/model/aabb.hpp"
#include "vkg/render/model/vertex.hpp"
#include "deferred/csm/csm_vert.hpp"

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

    if(!init) {
      init = true;

      frustums.resize(numCascades);
      cascades.resize(numCascades);
      cascadesBuffers.resize(ctx.numFrames);
      for(int i = 0; i < ctx.numFrames; ++i)
        cascadesBuffers[i] = buffer::devStorageBuffer(
          ctx.device, sizeof(CascadeDesc) * numCascades, toString("cascades_", i));
      resources.set(passOut.frustums, {frustums});
    }

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
    auto fov = camera->fov();
    auto camView = camera->view();

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
      cascades[i] = {lightViewProj, sunDir, f};
      //cull using camera near far frustum
      auto camFruProj = perspectiveMatrix(fov, a, n, f);
      frustums[i] = Frustum{camFruProj * camView};
    }
    resources.set(passOut.cascades, cascadesBuffers[ctx.frameIndex]->bufferInfo());
  }
  void execute(RenderContext &ctx, Resources &resources) override {
    auto cb = ctx.graphics;

    ctx.device.begin(cb, "update light frustums");
    auto bufInfo = cascadesBuffers[ctx.frameIndex]->bufferInfo();
    cb.updateBuffer(
      bufInfo.buffer, bufInfo.offset, sizeof(CascadeDesc) * cascades.size(),
      cascades.data());
    ctx.device.end(cb);

    cb.pipelineBarrier(
      vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eAllGraphics, {},
      vk::MemoryBarrier{
        vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead},
      nullptr, nullptr);
  }

private:
  std::vector<Frustum> frustums;
  std::vector<CascadeDesc> cascades;
  std::vector<std::unique_ptr<Buffer>> cascadesBuffers;
  bool init{false};
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
                      .maxPerGroup = passIn.maxPerGroup});

  cullPassOut = cull.out();
  cascades = frustum.out().cascades;

  builder.read(passIn.shadowMapSetting);
  builder.read(cascades);
  builder.read(cullPassOut);
  passOut = {
    .settingBuffer = builder.create<BufferInfo>("ShadowMapSetting"),
    .cascades = frustum.out().cascades,
    .shadowMaps = builder.create<Texture *>("ShadowMaps"),
  };
  return passOut;
}
void ShadowMapPass::compile(RenderContext &ctx, Resources &resources) {
  auto setting = resources.get(passIn.shadowMapSetting);

  if(!setting.isEnabled()) return;

  if(!init) {
    init = true;
    shadowMapSetting = buffer::hostUniformBuffer(ctx.device, sizeof(UBOShadowMapSetting));
    shadowMapSetting->ptr<UBOShadowMapSetting>()->numCascades = setting.numCascades();
    createPipeline(ctx.device, setting);
    createTextures(ctx.device, setting, ctx.numFrames);

    resources.set(passOut.settingBuffer, shadowMapSetting->bufferInfo());
  }

  auto &frame = frames[ctx.frameIndex];

  calcSetDef.cascades(resources.get(cascades));
  calcSetDef.matrices(resources.get(passIn.matrices));
  calcSetDef.update(frame.calcSet);

  resources.set(passOut.shadowMaps, frame.shadowMaps.get());
}

void ShadowMapPass::createTextures(
  Device &device, ShadowMapSetting &setting, uint32_t numFrames) {
  descriptorPool = DescriptorPoolMaker()
                     .pipelineLayout(calcPipeDef, numFrames)
                     .createUnique(device.vkDevice());

  frames.resize(numFrames);
  for(auto &frame: frames) {
    frame.calcSet = calcSetDef.createSet(*descriptorPool);

    frame.shadowMaps = image::make2DArrayTex(
      "shadowMaps", device, setting.textureSize(), setting.textureSize(),
      setting.numCascades(),
      vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
      vk::Format::eD32Sfloat, vk::SampleCountFlagBits::e1,
      vk::ImageAspectFlagBits::eDepth, false);
    frame.shadowMaps->setSampler(
      {{}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear});

    frame.shadowMapLayerViews.resize(setting.numCascades());
    std::vector<vk::ImageView> attachments(setting.numCascades());
    for(int c = 0; c < setting.numCascades(); ++c) {
      frame.shadowMapLayerViews[c] = frame.shadowMaps->createLayerImageView(c);
      attachments[c] = *frame.shadowMapLayerViews[c];
    }
    vk::FramebufferCreateInfo info{
      {},
      *renderPass,
      uint32_t(attachments.size()),
      attachments.data(),
      setting.textureSize(),
      setting.textureSize(),
      1};
    frame.framebuffer = device.vkDevice().createFramebufferUnique(info);
  }
}

auto ShadowMapPass::createPipeline(Device &device, ShadowMapSetting &setting) -> void {

  {
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

  {
    calcSetDef.init(device);

    calcPipeDef.set(calcSetDef);
    calcPipeDef.init(device);
  }

  {
    GraphicsPipelineMaker maker(device.vkDevice());

    maker.layout(calcPipeDef.layout())
      .renderPass(*renderPass)
      .vertexInputAuto(
        {{.stride = sizeof(Vertex::Position),
          .attributes = {{vk::Format::eR32G32B32Sfloat}}},
         {.stride = sizeof(Vertex::Normal),
          .attributes = {{vk::Format::eR32G32B32Sfloat}}},
         {.stride = sizeof(Vertex::UV), .attributes = {{vk::Format::eR32G32Sfloat}}}})
      .inputAssembly(vk::PrimitiveTopology::eTriangleList)
      .polygonMode(vk::PolygonMode::eFill)
      .cullMode(vk::CullModeFlagBits::eBack)
      .frontFace(vk::FrontFace::eCounterClockwise)
      .depthTestEnable(true)
      .depthWriteEnable(true)
      .depthCompareOp(vk::CompareOp::eLessOrEqual)
      .viewport({})
      .scissor({})
      .dynamicState(vk::DynamicState::eViewport)
      .dynamicState(vk::DynamicState::eScissor);

    maker.shader(
      vk::ShaderStageFlagBits::eVertex,
      Shader{shader::deferred::csm::csm_vert_span, setting.numCascades()});

    pipes.resize(setting.numCascades());
    for(int i = 0; i < setting.numCascades(); ++i) {
      maker.subpass(subpasses[i]);
      pipes[i] = maker.createUnique();
    }
  }
}

void ShadowMapPass::execute(RenderContext &ctx, Resources &resources) {
  auto cb = ctx.graphics;

  auto drawInfos = resources.get(cullPassOut.drawInfos);
  auto setting = resources.get(passIn.shadowMapSetting);

  auto &frame = frames[ctx.frameIndex];

  std::vector<vk::ClearValue> clearValues(setting.numCascades());
  for(int i = 0; i < setting.numCascades(); ++i)
    clearValues[i] = vk::ClearDepthStencilValue{1.0f, 0};
  vk::RenderPassBeginInfo renderPassBeginInfo{
    *renderPass, *frame.framebuffer,
    vk::Rect2D{{0, 0}, {setting.textureSize(), setting.textureSize()}},
    uint32_t(clearValues.size()), clearValues.data()};

  vk::Viewport viewport{
    0, 0, float(setting.textureSize()), float(setting.textureSize()), 0.0f, 1.0f};
  vk::Rect2D scissor{{0, 0}, {setting.textureSize(), setting.textureSize()}};

  ctx.device.begin(cb, "shadow map");

  cb.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
  cb.setViewport(0, viewport);
  cb.setScissor(0, scissor);

  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics, calcPipeDef.layout(), calcPipeDef.set.set(),
    frame.calcSet, nullptr);
  auto bufInfo = resources.get(passIn.positions);
  cb.bindVertexBuffers(0, bufInfo.buffer, bufInfo.offset);
  bufInfo = resources.get(passIn.normals);
  cb.bindVertexBuffers(1, bufInfo.buffer, bufInfo.offset);
  bufInfo = resources.get(passIn.uvs);
  cb.bindVertexBuffers(2, bufInfo.buffer, bufInfo.offset);
  bufInfo = resources.get(passIn.indices);
  cb.bindIndexBuffer(bufInfo.buffer, bufInfo.offset, vk::IndexType::eUint32);

  auto draw = [&](uint32_t frustumIdx, DrawGroup drawGroup) {
    auto drawGroupIdx = value(drawGroup);
    auto drawInfo = drawInfos.drawInfo[frustumIdx][drawGroupIdx];
    if(drawInfo.maxCount == 0) return;
    cb.drawIndexedIndirectCount(
      drawInfo.drawCMD.buffer, drawInfo.drawCMD.offset, drawInfo.drawCMDCount.buffer,
      drawInfo.drawCMDCount.offset, drawInfo.maxCount, drawInfo.stride);
  };
  for(auto i = 0u; i < setting.numCascades(); ++i) {
    pushContant.cascadeIndex = i;
    cb.pushConstants<PushContant>(
      calcPipeDef.layout(), vk::ShaderStageFlagBits::eVertex, 0, pushContant);

    cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipes[i]);
    draw(i, DrawGroup::BRDF);
    draw(i, DrawGroup::Reflective);
    draw(i, DrawGroup::Refractive);
    if(i + 1 < setting.numCascades()) cb.nextSubpass(vk::SubpassContents::eInline);
  }
  cb.endRenderPass();
  ctx.device.end(cb);
}
}