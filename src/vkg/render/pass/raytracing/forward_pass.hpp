#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/model/camera.hpp"
#include "vkg/render/model/vertex.hpp"
#include "vkg/render/draw_group.hpp"
#include "vkg/render/pass/cull/compute_cull_drawcmd.hpp"
#include "trace_rays_pass.hpp"

namespace vkg {
struct ForwardPassIn {
  TraceRaysPassOut traceRays;
  CamFrustumPassOut camFrustum;

  FrameGraphResource<SceneConfig> sceneConfig;
  FrameGraphResource<BufferInfo> meshInstances;
  FrameGraphResource<uint32_t> meshInstancesCount;
  FrameGraphResource<BufferInfo> primitives;
  FrameGraphResource<BufferInfo> positions;
  FrameGraphResource<BufferInfo> normals;
  FrameGraphResource<BufferInfo> uvs;
  FrameGraphResource<BufferInfo> indices;
  FrameGraphResource<BufferInfo> matrices;
  FrameGraphResource<BufferInfo> materials;
  FrameGraphResource<std::span<vk::DescriptorImageInfo>> samplers;
  FrameGraphResource<uint32_t> numValidSampler;
  FrameGraphResource<std::span<uint32_t>> countPerDrawGroup;
};
struct ForwardPassOut {
  FrameGraphResource<Texture *> hdrImg;
};
class ForwardPass: public Pass<ForwardPassIn, ForwardPassOut> {
public:
  void setup(PassBuilder &builder) override;
  void compile(RenderContext &ctx, Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  void createRenderPass(Device &device, vk::Format format);
  void createCopyDepthPass(Device &device, SceneConfig &sceneConfig);
  void createOpaquePass(Device &device, SceneConfig &sceneConfig);
  void createTransparentPass(Device &device, SceneConfig &sceneConfig);
  
  ComputeCullDrawCMDPassOut cullPassOut;

  struct SceneSetDef: DescriptorSetDef {
    __sampler2D__(depth, vkStage::eFragment);
    __buffer__(camera, vkStage::eVertex | vkStage::eFragment);
    __buffer__(meshInstances, vkStage::eVertex);
    __buffer__(primitives, vkStage::eVertex);
    __buffer__(matrices, vkStage::eVertex);
    __buffer__(materials, vkStage::eFragment);
    __sampler2D__(textures, vkStage::eFragment);
  } sceneSetDef;

  struct PushConstant {
    uint32_t frame;
  } pushConstant{};

  struct DeferredPipeDef: PipelineLayoutDef {
    __push_constant__(constant, vkStage::eVertex, PushConstant);
    __set__(scene, SceneSetDef);
  } pipeDef;

  float lineWidth_{1.f};

  bool init{false};

  uint32_t copyDepthPass{}, opaquePass, transparentPass{};
  vk::UniqueRenderPass renderPass;
  vk::UniquePipeline copyDepthPipe, opaqueLinesPipe, transparentPipe,
    transparentLinesPipe;

  vk::UniqueDescriptorPool descriptorPool;
  struct FrameResource {
    Texture *backImg;
    std::unique_ptr<Texture> depthAtt;
    uint64_t lastNumValidSampler{0};
    vk::DescriptorSet sceneSet;
    vk::UniqueFramebuffer framebuffer;
  };
  std::vector<FrameResource> frames;
};
}
