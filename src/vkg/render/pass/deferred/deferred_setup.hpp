#pragma once
#include "deferred.hpp"

namespace vkg {
struct DeferredSetupPassIn {
  FrameGraphResource<Texture *> backImg;
  FrameGraphResource<Camera *> camera;
  FrameGraphResource<SceneConfig> sceneConfig;
  FrameGraphResource<BufferInfo> meshInstances;
  FrameGraphResource<uint32_t> meshInstancesCount;
  FrameGraphResource<BufferInfo> positions;
  FrameGraphResource<BufferInfo> normals;
  FrameGraphResource<BufferInfo> uvs;
  FrameGraphResource<BufferInfo> indices;
  FrameGraphResource<BufferInfo> primitives;
  FrameGraphResource<BufferInfo> matrices;
  FrameGraphResource<BufferInfo> materials;
  FrameGraphResource<std::span<vk::DescriptorImageInfo>> samplers;
  FrameGraphResource<uint32_t> numValidSampler;
  FrameGraphResource<BufferInfo> lighting;
  FrameGraphResource<BufferInfo> lights;

  FrameGraphResource<std::span<uint32_t>> shadeModelCount;

  FrameGraphResource<AtmosphereSetting> atmosSetting;
  AtmospherePassOut atmosphere;

  FrameGraphResource<ShadowMapSetting> shadowMapSetting;
  ShadowMapPassOut shadowmap;
};
struct DeferredSetupPassOut {
  FrameGraphResource<Texture *> backImg;
};
class DeferredSetupPass: public Pass<DeferredSetupPassIn, DeferredSetupPassOut> {
public:
  void setup(PassBuilder &builder) override;
};
}