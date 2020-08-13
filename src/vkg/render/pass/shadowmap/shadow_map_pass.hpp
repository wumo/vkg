#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/model/shadow_map.hpp"
#include "vkg/math/glm_common.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/render/model/camera.hpp"
#include "vkg/render/draw_group.hpp"
#include "vkg/render/pass/compute_cull_drawcmd.hpp"

namespace vkg {
struct ShadowMapPassIn {
  FrameGraphResource<ShadowMapSetting> shadowMapSetting;
  FrameGraphResource<Camera *> camera;
  FrameGraphResource<vk::Buffer> cameraBuffer;
  FrameGraphResource<SceneConfig> sceneConfig;
  FrameGraphResource<vk::Buffer> meshInstances;
  FrameGraphResource<uint32_t> meshInstancesCount;
  FrameGraphResource<vk::Buffer> primitives;
  FrameGraphResource<vk::Buffer> matrices;
  FrameGraphResource<std::span<uint32_t>> maxPerGroup;
};
struct ShadowMapPassOut {
  FrameGraphResource<vk::Buffer> setting;
  FrameGraphResource<vk::Buffer> cascades;
  FrameGraphResource<Texture *> shadowMaps;
};

class ShadowMapPass: public Pass<ShadowMapPassIn, ShadowMapPassOut> {
public:
  auto setup(PassBuilder &builder, const ShadowMapPassIn &inputs)
    -> ShadowMapPassOut override;
  void compile(RenderContext &ctx, Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  auto createRenderPass(Device &device, ShadowMapSetting &setting) -> void;
  auto createDescriptorSets(Device &device, ShadowMapSetting &setting) -> void;
  auto createPipeline(Device &device, ShadowMapSetting &setting) -> void;
  auto createTextures(Device &device, ShadowMapSetting &setting) -> void;

  struct CalcSetDef: DescriptorSetDef {
    __buffer__(cascades, vk::ShaderStageFlagBits::eVertex);
    __buffer__(matrices, vk::ShaderStageFlagBits::eVertex);
  } calcSetDef;

  struct CalcPipeDef: PipelineLayoutDef {
    __push_constant__(cascadeIndex, vk::ShaderStageFlagBits::eVertex, uint32_t);
    __set__(set, CalcSetDef);
  } calcPipeDef;

  struct UBOShadowMapSetting {
    uint32_t numCascades;
  };

  struct CascadeDesc {
    glm::mat4 lightViewProj;
    glm::vec3 lightDir;
    float z;
  };

  ComputeCullDrawCMDPassOut cullPassOut;

  vk::UniqueDescriptorPool descriptorPool;
  vk::DescriptorSet calcSet;

  vk::UniqueRenderPass renderPass;
  std::vector<uint32_t> subpasses{};
  std::vector<vk::UniquePipeline> pipes;

  std::unique_ptr<Buffer> shadowMapSetting;
  std::unique_ptr<Buffer> cascades;
  std::vector<std::unique_ptr<Buffer>> frustums;
  std::unique_ptr<Texture> shadowMaps;
  std::vector<vk::UniqueImageView> shadowMapLayerViews;
  vk::UniqueFramebuffer framebuffer;
};
}
