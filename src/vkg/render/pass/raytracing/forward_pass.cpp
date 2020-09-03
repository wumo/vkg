#include "forward_pass.hpp"
namespace vkg {
void ForwardPass::setup(vkg::PassBuilder &builder) {
  //  auto cull = builder
  //                .newPass<ComputeCullDrawCMD>(
  //                  "Cull",
  //                  {
  //                    passIn.camFrustum.camFrustum,
  //                    passIn.meshInstances,
  //                    passIn.meshInstancesCount,
  //                    passIn.sceneConfig,
  //                    passIn.primitives,
  //                    passIn.matrices,
  //                    passIn.countPerDrawGroup,
  //                  })
  //                .out();

  passOut = {
    .hdrImg = passIn.traceRays.backImg,
  };
}
void ForwardPass::compile(vkg::RenderContext &ctx, vkg::Resources &resources) {}
void ForwardPass::execute(vkg::RenderContext &ctx, vkg::Resources &resources) {}
}