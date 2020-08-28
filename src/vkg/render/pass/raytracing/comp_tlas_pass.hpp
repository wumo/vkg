#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/render/graph/frame_graph.hpp"

namespace vkg {
struct CompTLASPassIn {
  FrameGraphResource<BufferInfo> meshInstances;
  FrameGraphResource<uint32_t> meshInstancesCount;
  FrameGraphResource<SceneConfig> sceneConfig;
  FrameGraphResource<BufferInfo> primitives;
  FrameGraphResource<BufferInfo> matrices;
  FrameGraphResource<std::span<uint32_t>> countPerDrawGroup;
};

struct CompTLASPassOut {
  FrameGraphResource<uint32_t> tlasCount;
  FrameGraphResource<BufferInfo> tlas;
};

class CompTLASPass: public Pass<CompTLASPassIn, CompTLASPassOut> {
public:
  void setup(PassBuilder &builder) override;
  void compile(RenderContext &ctx, Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  struct ComputeTLASSetDef: DescriptorSetDef {
    __buffer__(meshInstances, vkStage::eCompute);
    __buffer__(primitives, vkStage::eCompute);
    __buffer__(matrices, vkStage::eCompute);
    __buffer__(tlasInstanceCount, vkStage::eCompute);
    __buffer__(tlasInstances, vkStage::eCompute);
  } setDef;
  struct PushConstant {
    uint32_t totalMeshInstances;
    uint32_t frame;
  } pushConstant{};
  struct RTPipeDef: PipelineLayoutDef {
    __push_constant__(constant, vkStage::eCompute, PushConstant);
    __set__(set, ComputeTLASSetDef);
  } pipeDef;

  struct FrameResource {
    vk::DescriptorSet set;

    std::unique_ptr<Buffer> tlasInstanceCount;
    std::unique_ptr<Buffer> tlasInstances;
  };
  std::vector<FrameResource> frames;

  vk::UniqueDescriptorPool descriptorPool;
  vk::UniquePipeline pipe;
  const uint32_t local_size = 64;

  bool init{false};
};
}
