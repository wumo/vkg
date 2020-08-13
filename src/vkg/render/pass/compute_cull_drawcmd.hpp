#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/scene_config.hpp"

namespace vkg {
struct ComputeCullDrawCMDPassIn {
  FrameGraphResource<vk::Buffer> frustum;
  FrameGraphResource<vk::Buffer> meshInstances;
  FrameGraphResource<uint32_t> meshInstancesCount;
  FrameGraphResource<SceneConfig> sceneConfig;
  FrameGraphResource<vk::Buffer> primitives;
  FrameGraphResource<vk::Buffer> matrices;
  FrameGraphResource<std::vector<uint32_t>> drawGroupCount;
};
struct ComputeCullDrawCMDPassOut {
  FrameGraphResource<vk::Buffer> drawCMDBuffer;
  FrameGraphResource<vk::Buffer> drawCMDCountBuffer;
  FrameGraphResource<std::vector<uint32_t>> drawCMDOffsets;
  FrameGraphResource<uint32_t> drawCMDCountOffset;
};
class ComputeCullDrawCMD
  : public Pass<ComputeCullDrawCMDPassIn, ComputeCullDrawCMDPassOut> {
public:
  auto setup(PassBuilder &builder, const ComputeCullDrawCMDPassIn &inputs)
    -> ComputeCullDrawCMDPassOut override;
  void compile(RenderContext &ctx, Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  struct ComputeTransfSetDef: DescriptorSetDef {
    __uniform__(frustum, vk::ShaderStageFlagBits::eCompute);
    __buffer__(meshInstances, vk::ShaderStageFlagBits::eCompute);
    __buffer__(primitives, vk::ShaderStageFlagBits::eCompute);
    __buffer__(matrices, vk::ShaderStageFlagBits::eCompute);
    __buffer__(drawCMD, vk::ShaderStageFlagBits::eCompute);
    __buffer__(drawCMDOffset, vk::ShaderStageFlagBits::eCompute);
    __buffer__(drawCMDCount, vk::ShaderStageFlagBits::eCompute);
  } setDef;
  struct PushConstant {
    uint32_t totalMeshInstances;
    uint32_t drawCMDCountOffset;
  } pushConstant;
  struct ComputeTransfPipeDef: PipelineLayoutDef {
    __push_constant__(instCount, vk::ShaderStageFlagBits::eCompute, PushConstant);
    __set__(transf, ComputeTransfSetDef);
  } pipeDef;
  vk::UniqueDescriptorPool descriptorPool;
  vk::DescriptorSet set;
  vk::UniquePipeline pipe;
  const uint32_t local_size = 64;

  std::unique_ptr<Buffer> drawCMD;
  std::unique_ptr<Buffer> drawCMDOffsetBuffer;
  std::unique_ptr<Buffer> drawCMDCount;

  uint32_t numDrawCMDs;
  uint32_t numDrawCMDCounts;

  bool init{false};
};
}
