#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/math/glm_common.hpp"

namespace vkg {
struct ComputeTransfPassIn {
  FrameGraphResource<BufferInfo> transforms;
  FrameGraphResource<BufferInfo> meshInstances;
  FrameGraphResource<uint32_t> meshInstancesCount;
  FrameGraphResource<SceneConfig> sceneConfig;
};
struct ComputeTransfPassOut {
  FrameGraphResource<BufferInfo> matrices;
};

class ComputeTransf: public Pass<ComputeTransfPassIn, ComputeTransfPassOut> {
public:
  auto setup(PassBuilder &builder, const ComputeTransfPassIn &inputs)
    -> ComputeTransfPassOut override;
  void compile(RenderContext &ctx, Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  std::unique_ptr<Buffer> matrices;

  struct ComputeTransfSetDef: DescriptorSetDef {
    __buffer__(meshInstances, vkStage::eCompute);
    __buffer__(transforms, vkStage::eCompute);
    __buffer__(matrices, vkStage::eCompute);
  } setDef;
  struct ComputeTransfPipeDef: PipelineLayoutDef {
    __push_constant__(instCount, vkStage::eCompute, uint32_t);
    __set__(transf, ComputeTransfSetDef);
  } pipeDef;
  vk::UniqueDescriptorPool descriptorPool;
  vk::DescriptorSet set;
  vk::UniquePipeline pipe;
  const uint32_t local_size = 64;
};

}
