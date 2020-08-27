#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/math/frustum.hpp"

namespace vkg {
struct DrawInfo {
  BufferInfo drawCMD, drawCMDCount;
  uint32_t maxCount{0};
  uint32_t stride{sizeof(vk::DrawIndexedIndirectCommand)};
};
struct DrawInfos {
  std::vector<std::vector<DrawInfo>> drawInfo;
};
struct ComputeCullDrawCMDPassIn {
  FrameGraphResource<std::span<Frustum>> frustums;
  FrameGraphResource<BufferInfo> meshInstances;
  FrameGraphResource<uint32_t> meshInstancesCount;
  FrameGraphResource<SceneConfig> sceneConfig;
  FrameGraphResource<BufferInfo> primitives;
  FrameGraphResource<BufferInfo> matrices;
  FrameGraphResource<std::span<uint32_t>> maxPerGroup;
};
struct ComputeCullDrawCMDPassOut {
  FrameGraphResource<DrawInfos> drawInfos;
};
class ComputeCullDrawCMD
  : public Pass<ComputeCullDrawCMDPassIn, ComputeCullDrawCMDPassOut> {
public:
  void setup(PassBuilder &builder) override;
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
    uint32_t cmdFrustumStride;
    uint32_t groupStride;
    uint32_t frame;
  } pushConstant;
  struct ComputeTransfPipeDef: PipelineLayoutDef {
    __push_constant__(pushConst, vk::ShaderStageFlagBits::eCompute, PushConstant);
    __set__(transf, ComputeTransfSetDef);
  } pipeDef;

  vk::UniquePipeline pipe;
  const uint32_t local_size = 64;

  vk::UniqueDescriptorPool descriptorPool;

  struct FrameResource {
    vk::DescriptorSet set;

    std::unique_ptr<Buffer> frustumsBuf;
    std::unique_ptr<Buffer> drawCMD;
    std::unique_ptr<Buffer> cmdOffsetPerGroupBuffer;
    std::unique_ptr<Buffer> countOfGroupBuffer;
  };

  std::vector<FrameResource> frames;

  std::vector<uint32_t> cmdOffsetOfGroupInFrustum;

  uint32_t numFrustums{0};
  uint32_t numDrawCMDsPerFrustum;
  uint32_t numDrawGroups;

  bool init{false};
};
}
