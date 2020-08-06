#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/math/glm_common.hpp"

namespace vkg {
struct ComputeTransfPassIn {
  FrameGraphResource transforms;
  FrameGraphResource meshInstances;
  FrameGraphResource meshInstancesCount;
  FrameGraphResource maxNumMeshInstances;
};
struct ComputeTransfPassOut {
  FrameGraphResource matrices;
};

class ComputeTransf: public Pass<ComputeTransfPassIn, ComputeTransfPassOut> {
public:
  auto setup(PassBuilder &builder, const ComputeTransfPassIn &inputs)
    -> ComputeTransfPassOut override;
  void compile(Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  ComputeTransfPassIn passIn;
  ComputeTransfPassOut passOut;

  std::unique_ptr<Buffer> matrices;

  struct ComputeTransfSetDef: DescriptorSetDef {
    __buffer__(meshInstances, vk::ShaderStageFlagBits::eCompute);
    __buffer__(transforms, vk::ShaderStageFlagBits::eCompute);
    __buffer__(matrices, vk::ShaderStageFlagBits::eCompute);
  } setDef;
  struct ComputeTransfPipeDef: PipelineLayoutDef {
    __push_constant__(instCount, vk::ShaderStageFlagBits::eCompute, uint32_t);
    __set__(transf, ComputeTransfSetDef);
  } pipeDef;
  vk::UniqueDescriptorPool descriptorPool;
  vk::DescriptorSet set;
  vk::UniquePipeline pipe;
  const uint32_t local_size = 64;
};

}
