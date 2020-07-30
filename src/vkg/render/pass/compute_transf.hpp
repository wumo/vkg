#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"

namespace vkg {

class ComputeTransf: public Pass {
public:
  struct PassIn {
    FrameGraphResource transforms;
    FrameGraphResource meshInstances;
    FrameGraphResource meshInstancesCount;
    FrameGraphResource matrices;
  };
  struct PassOut {
    FrameGraphResource matrices;
  };

  explicit ComputeTransf(PassIn in);

  void setup(PassBuilder &builder) override;
  void compile(Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  PassIn in;

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

auto addComputeTransfPass(FrameGraph &builder, ComputeTransf::PassIn passIn)
  -> ComputeTransf::PassOut;
}
