#include "raytracing_pass.hpp"

namespace vkg {

void RayTracingPass::setup(PassBuilder &builder) {
  auto &compTLAS = builder.newPass<CompTLASPass>(
    "ComputeTLAS", {passIn.meshInstances, passIn.meshInstancesCount, passIn.sceneConfig,
                    passIn.primitives, passIn.matrices, passIn.countPerDrawGroup});
  compTlasPassOut = compTLAS.out();

  passOut = {
    .backImg = builder.write(passIn.backImg),
  };
}
void RayTracingPass::compile(RenderContext &ctx, Resources &resources) {}
void RayTracingPass::execute(RenderContext &ctx, Resources &resources) {}

}