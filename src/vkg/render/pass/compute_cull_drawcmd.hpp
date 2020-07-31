#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"

namespace vkg {
class ComputeCullDrawCMD: public Pass {
public:
  struct PassIn {
    FrameGraphResource frustum;
    FrameGraphResource meshInstances;
    FrameGraphResource meshInstancesCount;
    FrameGraphResource primitives;
    FrameGraphResource matrices;
    FrameGraphResource drawCMDBuffer;
    FrameGraphResource drawCMDCountBuffer;
    std::vector<FrameGraphResource> drawGroupCount;
  };
  struct PassOut {
    FrameGraphResource drawCMDBuffer;
    FrameGraphResource drawCMDCountBuffer;
  };

  explicit ComputeCullDrawCMD(PassIn passIn);

  void setup(PassBuilder &builder) override;
  void compile(Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  PassIn passIn;

  struct ComputeTransfSetDef: DescriptorSetDef {
    __uniform__(frustum, vk::ShaderStageFlagBits::eCompute);
    __buffer__(meshInstances, vk::ShaderStageFlagBits::eCompute);
    __buffer__(primitives, vk::ShaderStageFlagBits::eCompute);
    __buffer__(matrices, vk::ShaderStageFlagBits::eCompute);
    __buffer__(drawCMDOffset, vk::ShaderStageFlagBits::eCompute);
    __buffer__(drawCMD, vk::ShaderStageFlagBits::eCompute);
    __buffer__(drawCMDCount, vk::ShaderStageFlagBits::eCompute);
  } setDef;
  struct ComputeTransfPipeDef: PipelineLayoutDef {
    __push_constant__(instCount, vk::ShaderStageFlagBits::eCompute, uint32_t);
    __set__(transf, ComputeTransfSetDef);
  } pipeDef;
  vk::UniqueDescriptorPool descriptorPool;
  vk::DescriptorSet set;
  vk::UniquePipeline pipe;
  const uint32_t local_size = 64;

  std::unique_ptr<Buffer> drawCMDOffset;
};

auto addComputeCullDrawCMDPass(
  FrameGraph &builder, const ComputeCullDrawCMD::PassIn &passIn)
  -> ComputeCullDrawCMD::PassOut;
}
