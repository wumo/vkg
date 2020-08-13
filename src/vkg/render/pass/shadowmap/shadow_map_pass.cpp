#include "shadow_map_pass.hpp"
void vkg::ShadowMapPass::compile(vkg::RenderContext &ctx, vkg::Resources &resources) {
  BasePass::compile(ctx, resources);
}
void vkg::ShadowMapPass::execute(vkg::RenderContext &ctx, vkg::Resources &resources) {
  BasePass::execute(ctx, resources);
}
vkg::ShadowMapPassOut vkg::ShadowMapPass::setup(
  vkg::PassBuilder &builder, const vkg::ShadowMapPassIn &inputs) {
  return ShadowMapPassOut();
}
