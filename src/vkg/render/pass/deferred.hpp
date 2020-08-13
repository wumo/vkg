#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/model/camera.hpp"
#include "compute_cull_drawcmd.hpp"
#include "vkg/render/model/vertex.hpp"
#include "vkg/render/draw_group.hpp"
#include "atmosphere_pass.hpp"

namespace vkg {
struct DeferredPassIn {
  FrameGraphResource<Texture *> backImg;
  FrameGraphResource<Camera *> camera;
  FrameGraphResource<vk::Buffer> cameraBuffer;
  FrameGraphResource<SceneConfig> sceneConfig;
  FrameGraphResource<vk::Buffer> meshInstances;
  FrameGraphResource<uint32_t> meshInstancesCount;
  FrameGraphResource<vk::Buffer> positions;
  FrameGraphResource<vk::Buffer> normals;
  FrameGraphResource<vk::Buffer> uvs;
  FrameGraphResource<vk::Buffer> indices;
  FrameGraphResource<vk::Buffer> primitives;
  FrameGraphResource<vk::Buffer> matrices;
  FrameGraphResource<vk::Buffer> materials;
  FrameGraphResource<std::vector<vk::DescriptorImageInfo> *> samplers;
  FrameGraphResource<uint32_t> numValidSampler;
  FrameGraphResource<vk::Buffer> lighting;
  FrameGraphResource<vk::Buffer> lights;

  FrameGraphResource<std::vector<uint32_t>> drawGroupCount;

  AtmospherePassOut atmosphere;
};
struct DeferredPassOut {
  FrameGraphResource<Texture *> backImg;
};

class DeferredPass: public Pass<DeferredPassIn, DeferredPassOut> {
public:
  auto setup(PassBuilder &builder, const DeferredPassIn &inputs)
    -> DeferredPassOut override;
  void compile(RenderContext &ctx, Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  auto createAttachments(Device &device) -> void;
  auto createRenderPass(Device &device, vk::Format format) -> void;
  auto createGbufferPass(Device &device, SceneConfig sceneConfig) -> void;
  auto createLightingPass(Device &device, SceneConfig sceneConfig) -> void;
  auto createUnlitPass(Device &device, SceneConfig sceneConfig) -> void;
  auto createTransparentPass(Device &device, SceneConfig sceneConfig) -> void;

  DeferredPassIn passIn;
  DeferredPassOut passOut;
  ComputeCullDrawCMDPassOut cullPassOut;
  uint32_t lastNumValidSampler{0};

  struct SceneSetDef: DescriptorSetDef {
    __uniform__(cam, vkStage::eVertex | vkStage::eFragment);
    __buffer__(meshInstances, vkStage::eVertex);
    __buffer__(primitives, vkStage::eVertex);
    __buffer__(matrices, vkStage::eVertex);
    __buffer__(materials, vkStage::eFragment);
    __sampler2D__(textures, vkStage::eFragment);

    __uniform__(lighting, vkStage::eFragment);
    __buffer__(lights, vkStage::eFragment);
  } sceneSetDef;

  struct GBufferSetDef: DescriptorSetDef {
    __input__(position, vkStage::eFragment);
    __input__(normal, vkStage::eFragment);
    __input__(diffuse, vkStage::eFragment);
    __input__(specular, vkStage::eFragment);
    __input__(emissive, vkStage::eFragment);
    __input__(depth, vkStage::eFragment);
  } gbufferSetDef;

  struct CSMSetDef: DescriptorSetDef {
    __uniform__(setting, vkStage::eFragment);
    __uniform__(cascades, vkStage::eFragment);
    __sampler2DArray__(shadowMaps, vkStage::eFragment);
  } csmSetDef;

  struct AtmosphereSetDef: DescriptorSetDef {
    __uniform__(atmosphere, vkStage::eFragment);
    __uniform__(sun, vkStage::eFragment);
    __sampler3D__(transmittance, vkStage::eFragment);
    __sampler3D__(scattering, vkStage::eFragment);
    __sampler2D__(irradiance, vkStage::eFragment);
  } atmosphereSetDef;

  struct DeferredPipeDef: PipelineLayoutDef {
    __set__(scene, SceneSetDef);
    __set__(gbuffer, GBufferSetDef);
    __set__(atmosphere, AtmosphereSetDef);
    __set__(shadowMap, CSMSetDef);
  } deferredPipeDef;

  bool wireframe_{false};
  float lineWidth_{1.f};

  bool init{false};

  Texture *backImg_{nullptr};
  std::unique_ptr<Texture> backImgAtt, depthAtt, positionAtt, normalAtt, diffuseAtt,
    specularAtt, emissiveAtt;

  vk::UniqueDescriptorPool descriptorPool;
  vk::DescriptorSet sceneSet, gbSet, shadowMapSet, atmosphereSet;

  uint32_t gbPass{}, litPass{}, unlitPass{}, transPass{};
  vk::UniqueRenderPass renderPass;
  vk::UniquePipeline gbTriPipe, gbWireFramePipe;
  vk::UniquePipeline unlitTriPipe, unlitLinePipe;
  vk::UniquePipeline transTriPipe, transLinePipe;
  vk::UniquePipeline litPipe, litAtmosPipe, litCSMPipe, litAtmosCSMPipe;
  vk::UniqueFramebuffer framebuffer;
};
}
