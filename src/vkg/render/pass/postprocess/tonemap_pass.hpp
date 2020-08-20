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
};
}
