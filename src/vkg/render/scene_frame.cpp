#include "scene.hpp"
#include "pass/compute_transf.hpp"
#include "vkg/render/pass/deferred/deferred.hpp"
#include "vkg/render/pass/atmosphere/atmosphere_pass.hpp"
#include "vkg/render/pass/shadowmap/shadow_map_pass.hpp"

namespace vkg {

struct SceneSetupPassIn {
  FrameGraphResource<vk::Extent2D> swapchainExtent;
  FrameGraphResource<vk::Format> swapchainFormat;
  FrameGraphResource<uint64_t> swapchainVersion;
};
struct SceneSetupPassOut {
  FrameGraphResource<Texture *> backImg;
  FrameGraphResource<SceneConfig> sceneConfig;
  FrameGraphResource<vk::Buffer> positions;
  FrameGraphResource<vk::Buffer> normals;
  FrameGraphResource<vk::Buffer> uvs;
  FrameGraphResource<vk::Buffer> indices;
  FrameGraphResource<vk::Buffer> primitives;
  FrameGraphResource<vk::Buffer> materials;
  FrameGraphResource<vk::Buffer> transforms;
  FrameGraphResource<vk::Buffer> meshInstances;
  FrameGraphResource<uint32_t> meshInstancesCount;
  FrameGraphResource<vk::Buffer> lighting;
  FrameGraphResource<vk::Buffer> lights;
  FrameGraphResource<std::span<vk::DescriptorImageInfo>> samplers;
  FrameGraphResource<uint32_t> numValidSampler;
  FrameGraphResource<Camera *> camera;
  FrameGraphResource<vk::Buffer> cameraBuffer;
  FrameGraphResource<std::span<uint32_t>> maxPerGroup;
  FrameGraphResource<AtmosphereSetting> atmosphereSetting;
  FrameGraphResource<ShadowMapSetting> shadowMapSetting;
};

class SceneSetupPass: public Pass<SceneSetupPassIn, SceneSetupPassOut> {
public:
  explicit SceneSetupPass(Scene &scene): scene(scene) {}

  auto setup(PassBuilder &builder, const SceneSetupPassIn &inputs)
    -> SceneSetupPassOut override {
    passIn = inputs;
    builder.read(passIn.swapchainExtent);
    builder.read(passIn.swapchainFormat);
    builder.read(passIn.swapchainVersion);
    passOut = {
      .backImg = builder.create<Texture *>("backImg"),
      .sceneConfig = builder.create<SceneConfig>("sceneConfig"),
      .positions = builder.create<vk::Buffer>("positions"),
      .normals = builder.create<vk::Buffer>("normals"),
      .uvs = builder.create<vk::Buffer>("uvs"),
      .indices = builder.create<vk::Buffer>("indices"),
      .primitives = builder.create<vk::Buffer>("primitives"),
      .materials = builder.create<vk::Buffer>("materials"),
      .transforms = builder.create<vk::Buffer>("transforms"),
      .meshInstances = builder.create<vk::Buffer>("meshInstances"),
      .meshInstancesCount = builder.create<uint32_t>("meshInstancesCount"),
      .lighting = builder.create<vk::Buffer>("lighting"),
      .lights = builder.create<vk::Buffer>("lights"),
      .samplers = builder.create<std::span<vk::DescriptorImageInfo>>("textures"),
      .numValidSampler = builder.create<uint32_t>("numValidSampler"),
      .camera = builder.create<Camera *>("camera"),
      .cameraBuffer = builder.create<vk::Buffer>("cameraBuffer"),
      .maxPerGroup = builder.create<std::span<uint32_t>>("drawGroupCount"),
      .atmosphereSetting = builder.create<AtmosphereSetting>("atmosphere"),
      .shadowMapSetting = builder.create<ShadowMapSetting>("shadowMapSetting"),
    };
    return passOut;
  }
  void compile(RenderContext &ctx, Resources &resources) override {
    if(!boundPassData) {
      boundPassData = true;
      resources.set(passOut.sceneConfig, scene.sceneConfig);
      resources.set(passOut.positions, scene.Dev.positions->buffer());
      resources.set(passOut.normals, scene.Dev.normals->buffer());
      resources.set(passOut.uvs, scene.Dev.uvs->buffer());
      resources.set(passOut.indices, scene.Dev.indices->buffer());
      resources.set(passOut.primitives, scene.Dev.primitives->buffer());
      resources.set(passOut.materials, scene.Dev.materials->buffer());
      resources.set(passOut.transforms, scene.Dev.transforms->buffer());
      resources.set(passOut.meshInstances, scene.Dev.meshInstances->buffer());
      resources.set(passOut.lighting, scene.Dev.lighting->buffer());
      resources.set(passOut.lights, scene.Dev.lights->buffer());
      resources.set(passOut.camera, scene.Host.camera_.get());
      resources.set(passOut.cameraBuffer, scene.Dev.camera->buffer());
      resources.set(passOut.samplers, {scene.Dev.sampler2Ds});
    }
    resources.set(passOut.numValidSampler, uint32_t(scene.Dev.textures.size()));
    resources.set(passOut.atmosphereSetting, scene.atmosphere());
    resources.set(passOut.shadowMapSetting, scene.shadowmap());

    auto extent = resources.get<vk::Extent2D>(passIn.swapchainExtent);
    auto format = resources.get<vk::Format>(passIn.swapchainFormat);
    auto version = resources.get<uint64_t>(passIn.swapchainVersion);

    if(version > scene.swapchainVersion) {
      scene.swapchainVersion = version;
      using vkUsage = vk::ImageUsageFlagBits;
      scene.Dev.backImg = image::make2DTex(
        "backImg", scene.device, extent.width, extent.height,
        vkUsage::eSampled | vkUsage::eStorage | vkUsage::eTransferSrc |
          vkUsage ::eTransferDst | vkUsage ::eColorAttachment,
        format);
      resources.set(passOut.backImg, scene.Dev.backImg.get());
    }

    scene.Host.camera_->resize(extent.width, extent.height);
    scene.Host.camera_->updateUBO();

    resources.set(passOut.meshInstancesCount, scene.Dev.meshInstances->count());
    resources.set(passOut.maxPerGroup, {scene.Host.drawGroupInstCount});
  }

private:
  Scene &scene;
  bool boundPassData{false};
};

auto Scene::setup(PassBuilder &builder, const ScenePassIn &inputs) -> ScenePassOut {
  passIn = inputs;

  auto &sceneSetup = builder.newPass<SceneSetupPass>(
    "SceneSetup",
    SceneSetupPassIn{
      inputs.swapchainExtent, inputs.swapchainFormat, inputs.swapchainVersion},
    *this);

  auto &transf = builder.newPass<ComputeTransf>(
    "Transf", ComputeTransfPassIn{
                sceneSetup.out().transforms, sceneSetup.out().meshInstances,
                sceneSetup.out().meshInstancesCount, sceneSetup.out().sceneConfig});

  auto &atmosphere = builder.newPass<AtmospherePass>(
    "Atmosphere", AtmospherePassIn{sceneSetup.out().atmosphereSetting});

  auto &shadowMap = builder.newPass<ShadowMapPass>(
    "ShadowMap", ShadowMapPassIn{
                   sceneSetup.out().shadowMapSetting,
                   sceneSetup.out().camera,
                   sceneSetup.out().cameraBuffer,
                   sceneSetup.out().sceneConfig,
                   sceneSetup.out().meshInstances,
                   sceneSetup.out().meshInstancesCount,
                   sceneSetup.out().primitives,
                   transf.out().matrices,
                   sceneSetup.out().maxPerGroup,
                 });

  auto &deferred = builder.newPass<DeferredPass>(
    "Deferred", DeferredPassIn{
                  sceneSetup.out().backImg,
                  sceneSetup.out().camera,
                  sceneSetup.out().cameraBuffer,
                  sceneSetup.out().sceneConfig,
                  sceneSetup.out().meshInstances,
                  sceneSetup.out().meshInstancesCount,
                  sceneSetup.out().positions,
                  sceneSetup.out().normals,
                  sceneSetup.out().uvs,
                  sceneSetup.out().indices,
                  sceneSetup.out().primitives,
                  transf.out().matrices,
                  sceneSetup.out().materials,
                  sceneSetup.out().samplers,
                  sceneSetup.out().numValidSampler,
                  sceneSetup.out().lighting,
                  sceneSetup.out().lights,
                  sceneSetup.out().maxPerGroup,
                  sceneSetup.out().atmosphereSetting,
                  atmosphere.out(),
                  sceneSetup.out().shadowMapSetting,
                  shadowMap.out()});

  builder.read(passIn.swapchainExtent);
  passOut.backImg = deferred.out().backImg;
  passOut.renderArea = builder.create<vk::Rect2D>("renderArea");

  return passOut;
}

void Scene::compile(RenderContext &ctx, Resources &resources) {
  renderArea.extent = resources.get<vk::Extent2D>(passIn.swapchainExtent);
  resources.set(passOut.renderArea, renderArea);
}
}
