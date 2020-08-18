#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/model/camera.hpp"
#include "vkg/render/pass/compute_cull_drawcmd.hpp"
#include "vkg/render/model/vertex.hpp"
#include "vkg/render/draw_group.hpp"
#include "vkg/render/pass/atmosphere/atmosphere_pass.hpp"
#include "vkg/render/pass/shadowmap/shadow_map_pass.hpp"

namespace vkg {
struct DeferredPassIn {
  FrameGraphResource<uint64_t> backImgVersion;
  FrameGraphResource<std::span<Texture *>> backImgs;
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
  FrameGraphResource<uint32_t> transformStride;
  FrameGraphResource<BufferInfo> materials;
  FrameGraphResource<std::span<vk::DescriptorImageInfo>> samplers;
  FrameGraphResource<uint32_t> numValidSampler;
  FrameGraphResource<BufferInfo> lighting;
  FrameGraphResource<BufferInfo> lights;

  FrameGraphResource<std::span<uint32_t>> drawGroupCount;

  FrameGraphResource<AtmosphereSetting> atmosSetting;
  AtmospherePassOut atmosphere;

  FrameGraphResource<ShadowMapSetting> shadowMapSetting;
  ShadowMapPassOut shadowmap;
};
struct DeferredPassOut {
  FrameGraphResource<std::span<Texture *>> backImgs;
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

  ComputeCullDrawCMDPassOut cullPassOut;
  FrameGraphResource<BufferInfo> camBuffer;
  uint64_t lastNumValidSampler{0};
  uint64_t lastAtmosVersion{0};
  uint64_t lastShadowMapVersion{0};
  uint64_t lastBackImgVersion{0};

  struct SceneSetDef: DescriptorSetDef {
    __buffer__(cameras, vkStage::eVertex | vkStage::eFragment);
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
    __buffer__(cascades, vkStage::eFragment);
    __sampler2DArray__(shadowMaps, vkStage::eFragment);
  } csmSetDef;

  struct AtmosphereSetDef: DescriptorSetDef {
    __uniform__(atmosphere, vkStage::eFragment);
    __uniform__(sun, vkStage::eFragment);
    __sampler2D__(transmittance, vkStage::eFragment);
    __sampler3D__(scattering, vkStage::eFragment);
    __sampler2D__(irradiance, vkStage::eFragment);
  } atmosphereSetDef;

  struct PushContant {
    uint32_t transformStride;
    uint32_t frame;
  } pushContant;
  struct DeferredPipeDef: PipelineLayoutDef {
    __push_constant__(frame, vkStage::eVertex | vkStage ::eFragment, PushContant);
    __set__(scene, SceneSetDef);
    __set__(gbuffer, GBufferSetDef);
    __set__(atmosphere, AtmosphereSetDef);
    __set__(shadowMap, CSMSetDef);
  } deferredPipeDef;

  bool wireframe_{false};
  float lineWidth_{1.f};

  bool init{false};

  std::span<Texture *> backImgs_;
  std::vector<std::unique_ptr<Texture>> depthAtts, positionAtts, normalAtts, diffuseAtts,
    specularAtts, emissiveAtts;

  vk::UniqueDescriptorPool descriptorPool;
  vk::DescriptorSet sceneSet, shadowMapSet, atmosphereSet;

  std::vector<vk::DescriptorSet> gbSets;

  uint32_t gbPass{}, litPass{}, unlitPass{}, transPass{};
  vk::UniqueRenderPass renderPass;
  vk::UniquePipeline gbTriPipe, gbWireFramePipe;
  vk::UniquePipeline unlitTriPipe, unlitLinePipe;
  vk::UniquePipeline transTriPipe, transLinePipe;
  vk::UniquePipeline litPipe, litAtmosPipe, litCSMPipe, litAtmosCSMPipe;
  std::vector<vk::UniqueFramebuffer> framebuffers;
};
}
