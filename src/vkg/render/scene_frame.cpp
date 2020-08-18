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
  FrameGraphResource<BufferInfo> positions;
  FrameGraphResource<BufferInfo> normals;
  FrameGraphResource<BufferInfo> uvs;
  FrameGraphResource<BufferInfo> indices;
  FrameGraphResource<BufferInfo> primitives;
  FrameGraphResource<BufferInfo> materials;
  FrameGraphResource<BufferInfo> transforms;
  FrameGraphResource<BufferInfo> meshInstances;
  FrameGraphResource<uint32_t> meshInstancesCount;
  FrameGraphResource<BufferInfo> lighting;
  FrameGraphResource<BufferInfo> lights;
  FrameGraphResource<std::span<vk::DescriptorImageInfo>> samplers;
  FrameGraphResource<uint32_t> numValidSampler;
  FrameGraphResource<Camera *> camera;
  FrameGraphResource<BufferInfo> cameraBuffer;
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
      .positions = builder.create<BufferInfo>("positions"),
      .normals = builder.create<BufferInfo>("normals"),
      .uvs = builder.create<BufferInfo>("uvs"),
      .indices = builder.create<BufferInfo>("indices"),
      .primitives = builder.create<BufferInfo>("primitives"),
      .materials = builder.create<BufferInfo>("materials"),
      .transforms = builder.create<BufferInfo>("transforms"),
      .meshInstances = builder.create<BufferInfo>("meshInstances"),
      .meshInstancesCount = builder.create<uint32_t>("meshInstancesCount"),
      .lighting = builder.create<BufferInfo>("lighting"),
      .lights = builder.create<BufferInfo>("lights"),
      .samplers = builder.create<std::span<vk::DescriptorImageInfo>>("textures"),
      .numValidSampler = builder.create<uint32_t>("numValidSampler"),
      .camera = builder.create<Camera *>("camera"),
      .cameraBuffer = builder.create<BufferInfo>("cameraBuffer"),
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
      resources.set(passOut.positions, scene.Dev.positions->bufferInfo());
      resources.set(passOut.normals, scene.Dev.normals->bufferInfo());
      resources.set(passOut.uvs, scene.Dev.uvs->bufferInfo());
      resources.set(passOut.indices, scene.Dev.indices->bufferInfo());
      resources.set(passOut.primitives, scene.Dev.primitives->bufferInfo());
      resources.set(passOut.materials, scene.Dev.materials->bufferInfo());
      resources.set(passOut.transforms, scene.Dev.transforms->bufferInfo());
      resources.set(passOut.meshInstances, scene.Dev.meshInstances->bufferInfo());
      resources.set(passOut.lighting, scene.Dev.lighting->bufferInfo());
      resources.set(passOut.lights, scene.Dev.lights->bufferInfo());
      resources.set(passOut.camera, scene.Host.camera_.get());
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
    {inputs.swapchainExtent, inputs.swapchainFormat, inputs.swapchainVersion}, *this);

  auto &transf = builder.newPass<ComputeTransf>(
    "Transf", {sceneSetup.out().transforms, sceneSetup.out().meshInstances,
               sceneSetup.out().meshInstancesCount, sceneSetup.out().sceneConfig});

  auto &atmosphere =
    builder.newPass<AtmospherePass>("Atmosphere", {sceneSetup.out().atmosphereSetting});

  if(!sceneConfig.rayTraced) {
    auto &shadowMap = builder.newPass<ShadowMapPass>(
      "ShadowMap", {
                     sceneSetup.out().atmosphereSetting,
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
    shadowMap.enableIf([&]() { return Host.shadowMap.isEnabled(); });

    auto &deferred = builder.newPass<DeferredPass>(
      "Deferred", {sceneSetup.out().backImg,
                   sceneSetup.out().camera,
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
  }
  passOut.renderArea = builder.create<vk::Rect2D>("renderArea");

  return passOut;
}

void Scene::compile(RenderContext &ctx, Resources &resources) {
  renderArea.extent = resources.get<vk::Extent2D>(passIn.swapchainExtent);
  resources.set(passOut.renderArea, renderArea);
}
}
