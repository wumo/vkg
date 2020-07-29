#include "scene.hpp"

namespace vkg {

auto Scene::resize(uint32_t width, uint32_t height) -> void {
  Host.camera_->resize(width, height);
}

void Scene::setup(PassBuilder &builder) {
  out.positions = builder.create(toString(name, "_positions"));
  out.normals = builder.create(toString(name, "_normals"));
  out.uvs = builder.create(toString(name, "_uvs"));
  out.indices = builder.create(toString(name, "_indices"));
  out.primitives = builder.create(toString(name, "_primitives"));
  out.materials = builder.create(toString(name, "_materials"));
  out.transforms = builder.create(toString(name, "_transforms"));
  out.meshInstances = builder.create(toString(name, "_meshInstances"));
  out.matrices = builder.create(toString(name, "_matrices"));
  out.lighting = builder.create(toString(name, "_lighting"));
  out.lights = builder.create(toString(name, "_lights"));
  out.camera = builder.create(toString(name, "_camera"));
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
}
void Scene::execute(RenderContext &ctx, Resources &resources) {}

auto Scene::render() -> void {}
}
