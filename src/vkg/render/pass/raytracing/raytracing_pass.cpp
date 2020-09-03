#include "raytracing_pass.hpp"

namespace vkg {
void RayTracingPass::setup(PassBuilder &builder) {
  auto cam = builder
               .newPass<CamFrustumPass>(
                 "CamFrustumPass",
                 {
                   passIn.camera,
                 })
               .out();

  auto compTLAS = builder
                    .newPass<CompTLASPass>(
                      "ComputeTLAS",
                      {
                        passIn.meshInstances,
                        passIn.meshInstancesCount,
                        passIn.sceneConfig,
                        passIn.primitives,
                        passIn.matrices,
                        passIn.countPerDrawGroup,
                      })
                    .out();

  auto traceRays = builder
                     .newPass<TraceRaysPass>(
                       "TraceRays",
                       {
                         passIn.backImg,
                         cam.camBuffer,
                         compTLAS,
                         passIn.sceneConfig,
                         passIn.meshInstances,
                         passIn.positions,
                         passIn.normals,
                         passIn.uvs,
                         passIn.indices,
                         passIn.primitives,
                         passIn.materials,
                         passIn.samplers,
                         passIn.numValidSampler,
                         passIn.lighting,
                         passIn.lights,
                         passIn.atmosSetting,
                         passIn.atmosphere,
                       })
                     .out();

  auto forward = builder
                   .newPass<ForwardPass>(
                     "ForwardShading",
                     {
                       traceRays,
                       cam,
                       passIn.sceneConfig,
                       passIn.meshInstances,
                       passIn.meshInstancesCount,
                       passIn.primitives,
                       passIn.positions,
                       passIn.normals,
                       passIn.uvs,
                       passIn.indices,
                       passIn.matrices,
                       passIn.materials,
                       passIn.samplers,
                       passIn.numValidSampler,
                       passIn.countPerDrawGroup,
                     })
                   .out();

  passOut = {
    .backImg = forward.hdrImg,
  };
}
}