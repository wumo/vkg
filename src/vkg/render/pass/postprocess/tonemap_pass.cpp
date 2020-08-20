#include "tonemap_pass.hpp"
namespace vkg {
void ToneMapPass::setup(PassBuilder &builder) {}
void ToneMapPass::compile(RenderContext &ctx, Resources &resources) {
  BasePass::compile(ctx, resources);
}
void ToneMapPass::execute(RenderContext &ctx, Resources &resources) {
  BasePass::execute(ctx, resources);
}
}