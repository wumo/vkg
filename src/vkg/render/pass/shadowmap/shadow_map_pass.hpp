#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/model/shadow_map.hpp"

namespace vkg {
struct ShadowMapPassIn {
  FrameGraphResource<ShadowMap> shadowmap;
};
struct ShadowMapPassOut {
  FrameGraphResource<vk::Buffer> setting;
  FrameGraphResource<vk::Buffer> cascades;
  FrameGraphResource<Texture *> shadowMaps;
};

class ShadowMapPass: public Pass<ShadowMapPassIn, ShadowMapPassOut> {
public:
  auto setup(PassBuilder &builder, const ShadowMapPassIn &inputs)
    -> ShadowMapPassOut override;
  void compile(RenderContext &ctx, Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
};
}
