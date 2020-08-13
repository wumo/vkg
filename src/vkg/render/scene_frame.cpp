#include "scene.hpp"
#include "pass/compute_transf.hpp"
#include "vkg/render/pass/atmosphere/atmosphere_pass.hpp"
#include "vkg/render/pass/deferred/deferred.hpp"

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
  FrameGraphResource<std::vector<vk::DescriptorImageInfo> *> samplers;
  FrameGraphResource<uint32_t> numValidSampler;
  FrameGraphResource<Camera *> camera;
  FrameGraphResource<vk::Buffer> cameraBuffer;
  FrameGraphResource<std::vector<uint32_t>> drawGroupCount;
  FrameGraphResource<Atmosphere> atmosphere;
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
    passOut.backImg = builder.create<Texture *>("backImg");
    passOut.sceneConfig = builder.create<SceneConfig>("sceneConfig");
    passOut.positions = builder.create<vk::Buffer>("positions");
    passOut.normals = builder.create<vk::Buffer>("normals");
    passOut.uvs = builder.create<vk::Buffer>("uvs");
    passOut.indices = builder.create<vk::Buffer>("indices");
    passOut.primitives = builder.create<vk::Buffer>("primitives");
    passOut.materials = builder.create<vk::Buffer>("materials");
    passOut.transforms = builder.create<vk::Buffer>("transforms");
    passOut.meshInstances = builder.create<vk::Buffer>("meshInstances");
    passOut.meshInstancesCount = builder.create<uint32_t>("meshInstancesCount");
    passOut.lighting = builder.create<vk::Buffer>("lighting");
    passOut.lights = builder.create<vk::Buffer>("lights");
    passOut.samplers = builder.create<std::vector<vk::DescriptorImageInfo> *>("textures");
    passOut.numValidSampler = builder.create<uint32_t>("numValidSampler");
    passOut.camera = builder.create<Camera *>("camera");
    passOut.cameraBuffer = builder.create<vk::Buffer>("cameraBuffer");
    passOut.drawGroupCount = builder.create<std::vector<uint32_t>>("drawGroupCount");
    passOut.atmosphere = builder.create<Atmosphere>("atmosphere");

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
      resources.set(passOut.samplers, &scene.Dev.sampler2Ds);
    }
    resources.set(passOut.numValidSampler, uint32_t(scene.Dev.textures.size()));
    resources.set(passOut.atmosphere, scene.atmosphere());

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
    resources.set(passOut.drawGroupCount, scene.Host.drawGroupInstCount);
  }

private:
  Scene &scene;
  SceneSetupPassIn passIn;
  SceneSetupPassOut passOut;
  bool boundPassData{false};
};

auto Scene::setup(PassBuilder &builder, const ScenePassIn &inputs) -> ScenePassOut {
  passIn = inputs;

  auto sceneSetupPassOut = builder.newPass<SceneSetupPass>(
    "SceneSetup",
    SceneSetupPassIn{
      inputs.swapchainExtent, inputs.swapchainFormat, inputs.swapchainVersion},
    *this);

  auto transfPassOut = builder.newPass<ComputeTransf>(
    "Transf", ComputeTransfPassIn{
                sceneSetupPassOut.transforms, sceneSetupPassOut.meshInstances,
                sceneSetupPassOut.meshInstancesCount, sceneSetupPassOut.sceneConfig});

  auto atmospherePassOut = builder.newPass<AtmospherePass>(
    "Atmosphere", AtmospherePassIn{sceneSetupPassOut.atmosphere});

  auto deferredPassOut = builder.newPass<DeferredPass>(
    "Deferred", DeferredPassIn{
                  sceneSetupPassOut.backImg,
                  sceneSetupPassOut.camera,
                  sceneSetupPassOut.cameraBuffer,
                  sceneSetupPassOut.sceneConfig,
                  sceneSetupPassOut.meshInstances,
                  sceneSetupPassOut.meshInstancesCount,
                  sceneSetupPassOut.positions,
                  sceneSetupPassOut.normals,
                  sceneSetupPassOut.uvs,
                  sceneSetupPassOut.indices,
                  sceneSetupPassOut.primitives,
                  transfPassOut.matrices,
                  sceneSetupPassOut.materials,
                  sceneSetupPassOut.samplers,
                  sceneSetupPassOut.numValidSampler,
                  sceneSetupPassOut.lighting,
                  sceneSetupPassOut.lights,
                  sceneSetupPassOut.drawGroupCount,
                  sceneSetupPassOut.atmosphere,
                  atmospherePassOut});

  builder.read(passIn.swapchainExtent);
  passOut.backImg = deferredPassOut.backImg;
  passOut.renderArea = builder.create<vk::Rect2D>("renderArea");

  return passOut;
}

void Scene::compile(RenderContext &ctx, Resources &resources) {
  renderArea.extent = resources.get<vk::Extent2D>(passIn.swapchainExtent);
  resources.set(passOut.renderArea, renderArea);
}
}
