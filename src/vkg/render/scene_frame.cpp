#include "scene.hpp"
#include "pass/compute_transf.hpp"
#include "pass/deferred.hpp"

namespace vkg {

auto Scene::setup(PassBuilder &builder, const ScenePassIn &inputs) -> ScenePassOut {
  passIn = inputs;

  using ty = ResourceType;
  builder.read(passIn.swapchainExtent);
  passOut.positions = builder.create("_positions", ty::eBuffer);
  passOut.normals = builder.create("_normals", ty::eBuffer);
  passOut.uvs = builder.create("_uvs", ty::eBuffer);
  passOut.indices = builder.create("_indices", ty::eBuffer);
  passOut.primitives = builder.create("_primitives", ty::eBuffer);
  passOut.materials = builder.create("_materials", ty::eBuffer);
  passOut.transforms = builder.create("_transforms", ty::eBuffer);
  passOut.meshInstances = builder.create("_meshInstances", ty::eBuffer);
  passOut.meshInstancesCount = builder.create("_meshInstancesCount", ty::eValue);
  passOut.maxNumMeshInstances = builder.create("_maxNumMeshInstances", ty::eValue);
  passOut.lighting = builder.create("_lighting", ty::eBuffer);
  passOut.lights = builder.create("_lights", ty::eBuffer);
  passOut.textures = builder.create("_textures", ty::eValue);
  passOut.camera = builder.create("_camera", ty::eBuffer);
  passOut.cameraBuffer = builder.create("_cameraBuffer", ty::eValue);
  passOut.drawGroupCount = builder.create("_drawGroupCount", ty::eValue);

  auto transfPassOut =
    builder.newPass<ComputeTransf, ComputeTransfPassIn, ComputeTransfPassOut>(
      "ComputeTransf", {passOut.transforms, passOut.meshInstances,
                        passOut.meshInstancesCount, passOut.maxNumMeshInstances});

  auto deferredPassOut = builder.newPass<DeferredPass, DeferredPassIn, DeferredPassOut>(
    "DeferredPass",
    {passOut.camera, passOut.cameraBuffer, passOut.meshInstances,
     passOut.meshInstancesCount, passOut.maxNumMeshInstances, passOut.primitives,
     transfPassOut.matrices, passOut.materials, passOut.textures, passOut.lighting,
     passOut.lights, passOut.drawGroupCount});
  return passOut;
}

void Scene::compile(Resources &resources) {
  if(!boundPassData) {
    boundPassData = true;
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
    resources.set(passOut.maxNumMeshInstances, sceneConfig.maxNumMeshInstances);
    resources.set(passOut.textures, &Dev.sampler2Ds);
  }

  auto extent = resources.get<vk::Extent2D>(passIn.swapchainExtent);
  Host.camera_->resize(extent.width, extent.height);
  Host.camera_->updateUBO();

  resources.set(passOut.meshInstancesCount, Dev.meshInstances->count());
  resources.set(passOut.drawGroupCount, Host.drawGroupInstCount);
}
void Scene::execute(RenderContext &ctx, Resources &resources) {}
}
