#include "raytracing_pass.hpp"
namespace vkg {
auto RayTracingPass::setup(PassBuilder &builder, const RayTracingPassIn &inputs)
  -> RayTracingPassOut {
  return passOut;
}
void RayTracingPass::compile(RenderContext &ctx, Resources &resources) {}
void RayTracingPass::execute(RenderContext &ctx, Resources &resources) {}

}