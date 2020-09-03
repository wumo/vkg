#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/model/camera.hpp"
#include "vkg/render/model/vertex.hpp"
#include "vkg/render/draw_group.hpp"
#include "vkg/render/pass/cull/compute_cull_drawcmd.hpp"

namespace vkg {
struct ForwardPassIn {
  FrameGraphResource<Texture *> hdrImg;
  FrameGraphResource<Texture *> depthImg;

  FrameGraphResource<BufferInfo> camBuffer;
  FrameGraphResource<SceneConfig> sceneConfig;
  FrameGraphResource<BufferInfo> meshInstances;
  FrameGraphResource<uint32_t> meshInstancesCount;
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
  ComputeCullDrawCMDPassOut cullPassOut;
  FrameGraphResource<BufferInfo> camBuffer;

  struct SceneSetDef: DescriptorSetDef {
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
  vk::UniquePipeline copyDepthPipe, opaquePipe, transparentPipe;

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
