#include "scene.hpp"
#include "pass/compute_transf.hpp"
#include "pass/deferred.hpp"

namespace vkg {

auto Scene::setup(PassBuilder &builder, const ScenePassIn &inputs) -> ScenePassOut {
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

  auto transfPassOut = builder.newPass<ComputeTransf>(
    "Transf", ComputeTransfPassIn{
                passOut.transforms, passOut.meshInstances, passOut.meshInstancesCount,
                passOut.sceneConfig});

  auto deferredPassOut = builder.newPass<DeferredPass>(
    "Deferred",
    DeferredPassIn{
      passOut.backImg, passOut.camera, passOut.cameraBuffer, passOut.sceneConfig,
      passOut.meshInstances, passOut.meshInstancesCount, passOut.primitives,
      transfPassOut.matrices, passOut.materials, passOut.samplers,
      passOut.numValidSampler, passOut.lighting, passOut.lights, passOut.drawGroupCount});
  return passOut;
}

void Scene::compile(Resources &resources) {
  if(!boundPassData) {
    boundPassData = true;
    resources.set(passOut.sceneConfig, sceneConfig);
    resources.set(passOut.positions, Dev.positions->buffer());
    resources.set(passOut.normals, Dev.normals->buffer());
    resources.set(passOut.uvs, Dev.uvs->buffer());
    resources.set(passOut.indices, Dev.indices->buffer());
    resources.set(passOut.primitives, Dev.primitives->buffer());
    resources.set(passOut.materials, Dev.materials->buffer());
    resources.set(passOut.transforms, Dev.transforms->buffer());
    resources.set(passOut.meshInstances, Dev.meshInstances->buffer());
    resources.set(passOut.lighting, Dev.lighting->buffer());
    resources.set(passOut.lights, Dev.lights->buffer());
    resources.set(passOut.camera, Host.camera_.get());
    resources.set(passOut.cameraBuffer, Dev.camera->buffer());

    resources.set(passOut.samplers, &Dev.sampler2Ds);
  }
  resources.set(passOut.numValidSampler, uint32_t(Dev.textures.size()));

  auto extent = resources.get<vk::Extent2D>(passIn.swapchainExtent);
  auto format = resources.get<vk::Format>(passIn.swapchainFormat);
  auto version = resources.get<uint64_t>(passIn.swapchainVersion);

  if(version > swapchainVersion) {
    swapchainVersion = version;
    using vkUsage = vk::ImageUsageFlagBits;
    Dev.backImg = image::make2DTex(
      "backImg", device, extent.width, extent.height,
      vkUsage::eSampled | vkUsage::eStorage | vkUsage::eTransferSrc |
        vkUsage ::eTransferDst | vkUsage ::eColorAttachment,
      format);
    resources.set(passOut.backImg, Dev.backImg.get());
  }

  Host.camera_->resize(extent.width, extent.height);
  Host.camera_->updateUBO();

  resources.set(passOut.meshInstancesCount, Dev.meshInstances->count());
  resources.set(passOut.drawGroupCount, Host.drawGroupInstCount);
}
void Scene::execute(RenderContext &ctx, Resources &resources) {}
}
