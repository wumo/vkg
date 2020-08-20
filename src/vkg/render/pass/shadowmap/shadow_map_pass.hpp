#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/model/shadow_map.hpp"
#include "vkg/math/glm_common.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/render/model/camera.hpp"
#include "vkg/render/draw_group.hpp"
#include "vkg/render/pass/cull/compute_cull_drawcmd.hpp"
#include "vkg/render/pass/atmosphere/atmosphere_pass.hpp"

namespace vkg {
struct ShadowMapPassIn {
  FrameGraphResource<AtmosphereSetting> atmosSetting;
  FrameGraphResource<ShadowMapSetting> shadowMapSetting;
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
  FrameGraphResource<std::span<uint32_t>> maxPerGroup;
};
struct ShadowMapPassOut {
  FrameGraphResource<BufferInfo> settingBuffer;
  FrameGraphResource<BufferInfo> cascades;
  FrameGraphResource<Texture *> shadowMaps;
};

class ShadowMapPass: public Pass<ShadowMapPassIn, ShadowMapPassOut> {
public:
  auto setup(PassBuilder &builder, const ShadowMapPassIn &inputs)
    -> ShadowMapPassOut override;
  void compile(RenderContext &ctx, Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  auto createPipeline(Device &device, ShadowMapSetting &setting) -> void;
  void createTextures(Device &device, ShadowMapSetting &setting, uint32_t i);

  struct CalcSetDef: DescriptorSetDef {
    __buffer__(cascades, vk::ShaderStageFlagBits::eVertex);
    __buffer__(matrices, vk::ShaderStageFlagBits::eVertex);
  } calcSetDef;

  struct PushContant {
    uint32_t cascadeIndex;
  } pushContant;
  struct CalcPipeDef: PipelineLayoutDef {
    __push_constant__(cascadeIndex, vk::ShaderStageFlagBits::eVertex, PushContant);
    __set__(set, CalcSetDef);
  } calcPipeDef;

  struct UBOShadowMapSetting {
    uint32_t numCascades;
  };

  ComputeCullDrawCMDPassOut cullPassOut;
  FrameGraphResource<BufferInfo> cascades;

  vk::UniqueDescriptorPool descriptorPool;

  vk::UniqueRenderPass renderPass;
  std::vector<uint32_t> subpasses{};
  std::vector<vk::UniquePipeline> pipes;

  struct FrameResource {
    vk::DescriptorSet calcSet;
    std::unique_ptr<Texture> shadowMaps;
    std::vector<vk::UniqueImageView> shadowMapLayerViews;
    vk::UniqueFramebuffer framebuffer;
  };
  std::unique_ptr<Buffer> shadowMapSetting;
  std::vector<FrameResource> frames;
  bool init{false};
};
}
