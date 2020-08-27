#include "scene.hpp"
#include "vkg/render/pass/transf/compute_transf.hpp"
#include "vkg/render/pass/deferred/deferred.hpp"
#include "vkg/render/pass/atmosphere/atmosphere_pass.hpp"
#include "vkg/render/pass/shadowmap/shadow_map_pass.hpp"
#include "vkg/render/pass/raytracing/raytracing_pass.hpp"
#include "vkg/render/pass/postprocess/tonemap_pass.hpp"
#include "vkg/render/pass/postprocess/fxaa_pass.hpp"

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
  FrameGraphResource<std::span<uint32_t>> maxPerGroup;
  FrameGraphResource<AtmosphereSetting> atmosphereSetting;
  FrameGraphResource<ShadowMapSetting> shadowMapSetting;
};

class SceneSetupPass: public Pass<SceneSetupPassIn, SceneSetupPassOut> {
public:
  explicit SceneSetupPass(Scene &scene): scene(scene) {}

  void setup(PassBuilder &builder) override {
    builder.read(passIn);
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
      .maxPerGroup = builder.create<std::span<uint32_t>>("drawGroupCount"),
      .atmosphereSetting = builder.create<AtmosphereSetting>("atmosphere"),
      .shadowMapSetting = builder.create<ShadowMapSetting>("shadowMapSetting"),
    };
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
    auto version = resources.get<uint64_t>(passIn.swapchainVersion);

    if(version > scene.swapchainVersion) {
      scene.swapchainVersion = version;
      scene.Dev.backImgs.resize(ctx.numFrames);
      backImgs.resize(ctx.numFrames);
      for(auto i = 0u; i < ctx.numFrames; ++i) {
        using vkUsage = vk::ImageUsageFlagBits;
        scene.Dev.backImgs[i] = image::make2DTex(
          toString("backImg_", i), scene.device, extent.width, extent.height,
          vkUsage::eSampled | vkUsage::eStorage | vkUsage::eTransferSrc |
            vkUsage ::eTransferDst | vkUsage ::eColorAttachment,
          vk::Format::eR16G16B16A16Sfloat);
        backImgs[i] = scene.Dev.backImgs[i].get();
        backImgs[i]->setSampler(
          {{}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear});
      }
    }

    scene.Host.camera_->resize(extent.width, extent.height);

    if(!scene.Host.updates.empty()) {
      /**
       * we have to flush the same update to all frames to make them consistent. Each
       * frameUpdatable's ticket tracks its update history. `frames` is the
       * total number of inconsistent frames that needs to be updated. Reset `frames`
       * when user just updates it.
       */
      auto fetchUpdable = [](Scene &scene_, Update &update) {
        FrameUpdatable *updatable;
        switch(update.type) {
          case Update::Type::Material: updatable = &scene_.material(update.id); break;
          case Update::Type::Light: updatable = &scene_.light(update.id); break;
          case Update::Type::InstanceTransf:
            updatable = &scene_.modelInstance(update.id);
            break;
        }
        return updatable;
      };

      auto &updates = scene.Host.updates;
      uint32_t i = 0;
      while(i < updates.size()) {
        auto update = updates[i];
        auto updatable = fetchUpdable(scene, update);
        if(update.frames == 0) {       //remove
          updatable->ticket = nullIdx; //clear ticket
          if(i == updates.size() - 1)  //already the last one, just pop back
            updates.pop_back();
          else { //otherwise remove by swap with the last one, modify last one's ticket
            updates[i] = updates.back();
            updates.pop_back();
            fetchUpdable(scene, updates[i])->ticket = i;
          }
          continue; //(because of swap, we have to revisit this, or the last one removed,
                    // loop will still terminate because of size decrement.)
        }
        update.frames--;
        updatable->updateDesc(ctx.frameIndex);
        i++;
      }
    }

    resources.set(passOut.meshInstancesCount, scene.Dev.meshInstances->count());
    resources.set(passOut.maxPerGroup, {scene.Host.drawGroupInstCount});
    resources.set(passOut.backImg, backImgs[ctx.frameIndex]);
  }

private:
  Scene &scene;
  bool boundPassData{false};
  std::vector<Texture *> backImgs;
};

void Scene::setup(PassBuilder &builder) {
  auto &sceneSetup = builder.newPass<SceneSetupPass>(
    "SceneSetup",
    {passIn.swapchainExtent, passIn.swapchainFormat, passIn.swapchainVersion}, *this);

  auto &transf = builder.newPass<ComputeTransf>(
    "Transf", {sceneSetup.out().transforms, sceneSetup.out().meshInstances,
               sceneSetup.out().meshInstancesCount, sceneSetup.out().sceneConfig});

  auto &atmosphere =
    builder.newPass<AtmospherePass>("Atmosphere", {sceneSetup.out().atmosphereSetting});

  FrameGraphResource<Texture *> backImg;
  if(sceneConfig.rayTrace) {
    auto &rayTracing = builder.newPass<RayTracingPass>(
      "RayTracing", {
                      sceneSetup.out().backImg,
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
                    });
    backImg = rayTracing.out().backImg;
  } else {
    auto &shadowMap = builder.newPass<ShadowMapPass>(
      "ShadowMap", {
                     sceneSetup.out().atmosphereSetting,
                     sceneSetup.out().shadowMapSetting,
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

    backImg = deferred.out().backImg;
  }
  auto &tonemap = builder.newPass<ToneMapPass>("ToneMap", {backImg});
  auto &fxaa = builder.newPass<FxaaPass>("FXAA", {tonemap.out().img});
  auto &last = fxaa;

  builder.read(passIn.swapchainExtent);
  passOut = {
    .backImg = last.out().img,
    .renderArea = builder.create<vk::Rect2D>("renderArea"),
  };
}

void Scene::compile(RenderContext &ctx, Resources &resources) {
  renderArea.extent = resources.get<vk::Extent2D>(passIn.swapchainExtent);
  resources.set(passOut.renderArea, renderArea);
}
}
