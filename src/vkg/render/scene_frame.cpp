#include "scene.hpp"
#include "pass/compute_transf.hpp"

namespace vkg {

auto Scene::addPass(FrameGraph &builder) -> void {
  builder.addPass(name + "Pass", *this);

  auto transfPassOut = addComputeTransfPass(
    builder, {out.transforms, out.meshInstances, out.meshInstancesCount, out.matrices});
}

auto Scene::resize(uint32_t width, uint32_t height) -> void {
  Host.camera_->resize(width, height);
}

void Scene::setup(PassBuilder &builder) {
  using ty = ResourceType;
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
  }
  Host.camera_->updateUBO();
  resources.set(out.meshInstancesCount, Dev.meshInstances->count());
}
void Scene::execute(RenderContext &ctx, Resources &resources) {}

auto Scene::render() -> void {}
}
