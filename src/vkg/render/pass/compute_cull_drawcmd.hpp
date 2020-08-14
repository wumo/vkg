#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/math/frustum.hpp"

namespace vkg {
struct ComputeCullDrawCMDPassIn {
  FrameGraphResource<std::span<Frustum>> frustums;
  FrameGraphResource<vk::Buffer> meshInstances;
  FrameGraphResource<uint32_t> meshInstancesCount;
  FrameGraphResource<SceneConfig> sceneConfig;
  FrameGraphResource<vk::Buffer> primitives;
  FrameGraphResource<vk::Buffer> matrices;
  FrameGraphResource<std::span<uint32_t>> maxPerGroup;
};
struct ComputeCullDrawCMDPassOut {
  FrameGraphResource<vk::Buffer> drawCMDBuffer;
  FrameGraphResource<vk::Buffer> drawGroupCountBuffer;
  FrameGraphResource<std::span<uint32_t>> cmdOffsetPerFrustum;
  FrameGraphResource<std::span<uint32_t>> cmdOffsetPerGroup;
  FrameGraphResource<uint32_t> countOffset;
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
    __buffer__(frustums, vk::ShaderStageFlagBits::eCompute);
    __buffer__(meshInstances, vk::ShaderStageFlagBits::eCompute);
    __buffer__(primitives, vk::ShaderStageFlagBits::eCompute);
    __buffer__(matrices, vk::ShaderStageFlagBits::eCompute);
    __buffer__(drawCMD, vk::ShaderStageFlagBits::eCompute);
    __buffer__(cmdOffsetPerGroup, vk::ShaderStageFlagBits::eCompute);
    __buffer__(drawCMDCount, vk::ShaderStageFlagBits::eCompute);
  } setDef;
  struct PushConstant {
    uint32_t totalFrustums;
    uint32_t totalMeshInstances;
    uint32_t cmdFrameStride;
    uint32_t cmdFrustumStride;
    uint32_t groupFrameStride;
    uint32_t frame;
  } pushConstant;
  struct ComputeTransfPipeDef: PipelineLayoutDef {
    __push_constant__(instCount, vk::ShaderStageFlagBits::eCompute, PushConstant);
    __set__(transf, ComputeTransfSetDef);
  } pipeDef;

  vk::UniqueDescriptorPool descriptorPool;
  vk::DescriptorSet set;
  vk::UniquePipeline pipe;
  const uint32_t local_size = 64;

  std::unique_ptr<Buffer> frustumsBuf;
  std::unique_ptr<Buffer> drawCMD;
  std::unique_ptr<Buffer> cmdOffsetPerGroupBuffer;
  std::unique_ptr<Buffer> countPerGroupBuffer;

  std::vector<uint32_t> cmdOffsetPerFrustum;
  std::vector<uint32_t> cmdOffsetPerGroup;
  std::vector<uint32_t> countOffsetPerGroup;

  uint32_t numFrustums{0};
  uint32_t numDrawCMDsPerFrustum;
  uint32_t numDrawGroups;

  bool init{false};
};
}
