#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"

namespace vkg {
struct ToneMapPassIn {
  FrameGraphResource<Texture *> backImg;
};
struct ToneMapPassOut {
  FrameGraphResource<Texture *> backImg;
};
class ToneMapPass: public Pass<ToneMapPassIn, ToneMapPassOut> {
public:
  void setup(PassBuilder &builder) override;
  void compile(RenderContext &ctx, Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  struct ToneMapSetDef: DescriptorSetDef {
    __image2D__(image, vkStage::eCompute);
  } setDef;
  struct PushConstant {
    float exposure{1.0};
  } pushConstant;
  struct ToneMapPipeDef: PipelineLayoutDef {
    __push_constant__(constant, vkStage::eCompute, PushConstant);
    __set__(set, ToneMapSetDef);
  } pipeDef;

  vk::UniquePipeline pipe;

  vk::UniqueDescriptorPool descriptorPool;

  struct FrameResource {
    vk::DescriptorSet set;
  };
  std::vector<FrameResource> frames;
  bool init{false};
};
}
