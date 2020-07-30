#include <utility>

#include "scene.hpp"
#include "pass/compute_transf.hpp"

namespace vkg {

auto Scene::addPass(FrameGraph &builder, PassIn in) -> void {
  passIn = std::move(in);
  builder.addPass(name + "Pass", *this);

  auto transfPassOut = addComputeTransfPass(
    builder, {out.transforms, out.meshInstances, out.meshInstancesCount, out.matrices});
}

void Scene::setup(PassBuilder &builder) {
  using ty = ResourceType;
  builder.read(passIn.swapchainExtent);
  out.positions = builder.create(toString(name, "_positions"), ty::eBuffer);
  out.normals = builder.create(toString(name, "_normals"), ty::eBuffer);
  out.uvs = builder.create(toString(name, "_uvs"), ty::eBuffer);
  out.indices = builder.create(toString(name, "_indices"), ty::eBuffer);
  out.primitives = builder.create(toString(name, "_primitives"), ty::eBuffer);
  out.materials = builder.create(toString(name, "_materials"), ty::eBuffer);
  out.transforms = builder.create(toString(name, "_transforms"), ty::eBuffer);
  out.meshInstances = builder.create(toString(name, "_meshInstances"), ty::eBuffer);
  out.meshInstancesCount =
    builder.create(toString(name, "_meshInstancesCount"), ty::eValue);
  out.matrices = builder.create(toString(name, "_matrices"), ty::eBuffer);
  out.lighting = builder.create(toString(name, "_lighting"), ty::eBuffer);
  out.lights = builder.create(toString(name, "_lights"), ty::eBuffer);
  out.camera = builder.create(toString(name, "_camera"), ty::eBuffer);

  out.drawCMDBuffer = builder.create(toString(name, "_drawCMDBuffer"), ty::eBuffer);
  out.drawCMDCountBuffer =
    builder.create(toString(name, "_drawCMDCountBuffer"), ty::eBuffer);
  for(int i = 0; i < Host.drawGroupInstCount.size(); ++i)
    out.drawGroupCount.push_back(
      builder.create(toString(name, "_drawGroupCount_", i), ty::eValue));
}

void Scene::compile(Resources &resources) {
  if(!boundPassData) {
    boundPassData = true;
    resources.set(out.positions, Dev.positions->buffer());
    resources.set(out.normals, Dev.normals->buffer());
    resources.set(out.uvs, Dev.uvs->buffer());
    resources.set(out.indices, Dev.indices->buffer());
    resources.set(out.primitives, Dev.primitives->buffer());
    resources.set(out.materials, Dev.materials->buffer());
    resources.set(out.transforms, Dev.transforms->buffer());
    resources.set(out.meshInstances, Dev.meshInstances->buffer());
    resources.set(out.matrices, Dev.matrices->buffer());
    resources.set(out.lighting, Dev.lighting->buffer());
    resources.set(out.lights, Dev.lights->buffer());
    resources.set(out.camera, Dev.camera->buffer());
    resources.set(out.drawCMDBuffer, Dev.drawCMD->buffer());
    resources.set(out.drawCMDCountBuffer, Dev.drawCMDCount->buffer());
  }

  auto extent = resources.get<vk::Extent2D>(passIn.swapchainExtent);
  Host.camera_->resize(extent.width, extent.height);
  Host.camera_->updateUBO();

  resources.set(out.meshInstancesCount, Dev.meshInstances->count());

  for(int i = 0; i < Host.drawGroupInstCount.size(); ++i)
    resources.set(out.drawGroupCount[i], Host.drawGroupInstCount[i]);

  memset(Dev.drawCMDCount->ptr<uint32_t>(), 0, Dev.drawCMDCount->size());
}
void Scene::execute(RenderContext &ctx, Resources &resources) {}

auto Scene::render() -> void {}
}
