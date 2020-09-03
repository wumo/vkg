#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/model/camera.hpp"
#include "vkg/render/pass/cull/compute_cull_drawcmd.hpp"
#include "vkg/render/model/vertex.hpp"
#include "vkg/render/draw_group.hpp"
#include "vkg/render/pass/common/cam_frustum_pass.hpp"
#include "vkg/render/pass/atmosphere/atmosphere_pass.hpp"
#include "vkg/render/pass/shadowmap/shadow_map_pass.hpp"

namespace vkg {
struct DeferredPassIn {
  FrameGraphResource<Texture *> backImg;
  FrameGraphResource<BufferInfo> camBuffer;
  ComputeCullDrawCMDPassOut cullCMD;
  FrameGraphResource<SceneConfig> sceneConfig;
  FrameGraphResource<BufferInfo> meshInstances;
  FrameGraphResource<BufferInfo> positions;
  FrameGraphResource<BufferInfo> normals;
  FrameGraphResource<BufferInfo> uvs;
  FrameGraphResource<BufferInfo> indices;
  FrameGraphResource<BufferInfo> matrices;
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
  FrameGraphResource<Texture *> backImg;
};

class DeferredPass: public Pass<DeferredPassIn, DeferredPassOut> {
public:
  void setup(PassBuilder &builder) override;
  void compile(RenderContext &ctx, Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  auto createAttachments(Device &device, uint32_t frameIdx) -> void;
  auto createRenderPass(Device &device, vk::Format format) -> void;
  auto createGbufferPass(Device &device, SceneConfig sceneConfig) -> void;
  auto createLightingPass(Device &device, SceneConfig sceneConfig) -> void;
  auto createUnlitPass(Device &device, SceneConfig sceneConfig) -> void;
  auto createTransparentPass(Device &device, SceneConfig sceneConfig) -> void;

  struct SceneSetDef: DescriptorSetDef {
    __buffer__(camera, vkStage::eVertex | vkStage::eFragment);
    __buffer__(meshInstances, vkStage::eVertex);
    __buffer__(matrices, vkStage::eVertex);
    __buffer__(materials, vkStage::eFragment);
    __sampler2D__(textures, vkStage::eFragment);

    __uniform__(lighting, vkStage::eFragment);
    __buffer__(lights, vkStage::eFragment);
  } sceneSetDef;

  struct GBufferSetDef: DescriptorSetDef {
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

  struct PushConstant {
    uint32_t frame;
  } pushConstant{};
  struct DeferredPipeDef: PipelineLayoutDef {
    __push_constant__(constant, vkStage::eVertex, PushConstant);
    __set__(scene, SceneSetDef);
    __set__(gbuffer, GBufferSetDef);
    __set__(atmosphere, AtmosphereSetDef);
    __set__(shadowMap, CSMSetDef);
  } pipeDef;

  bool wireframe_{false};
  float lineWidth_{1.f};

  bool init{false};

  uint32_t gbPass{}, litPass{}, unlitPass{}, transPass{};
  vk::UniqueRenderPass renderPass;
  vk::UniquePipeline gbTriPipe, gbWireFramePipe;
  vk::UniquePipeline unlitTriPipe, unlitLinePipe;
  vk::UniquePipeline transTriPipe, transLinePipe;
  vk::UniquePipeline litPipe, litAtmosPipe, litCSMPipe, litAtmosCSMPipe;

  vk::UniqueDescriptorPool descriptorPool;

  std::vector<bool> groupCounted;
  struct FrameResource {
    Texture *backImg;
    std::unique_ptr<Texture> depthAtt, normalAtt, diffuseAtt, specularAtt, emissiveAtt;
    uint64_t lastNumValidSampler{0};
    vk::DescriptorSet sceneSet, gbSet, shadowMapSet, atmosphereSet;
    vk::UniqueFramebuffer framebuffer;
  };
  std::vector<FrameResource> frames;
};
}
