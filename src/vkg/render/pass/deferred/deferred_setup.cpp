#include "deferred_setup.hpp"
namespace vkg {

void DeferredSetupPass::setup(vkg::PassBuilder &builder) {
  auto cam = builder
               .newPass<CamFrustumPass>(
                 "CamFrustum",
                 {
                   passIn.camera,
                 })
               .out();

  auto cull = builder
                .newPass<ComputeCullDrawCMD>(
                  "CullDrawCMD",
                  {
                    cam.camFrustum,
                    passIn.meshInstances,
                    passIn.meshInstancesCount,
                    passIn.sceneConfig,
                    passIn.primitives,
                    passIn.matrices,
                    passIn.drawGroupCount,
                  },
                  std::set{
                    DrawGroup::Unlit,
                    DrawGroup::BRDF,
                    DrawGroup::Reflective,
                    DrawGroup::Refractive,
                    DrawGroup::Transparent,
                    DrawGroup::TransparentLines,
                    DrawGroup::OpaqueLines,
                  })
                .out();

  auto deferred = builder
                    .newPass<DeferredPass>(
                      "DeferredShading",
                      {
                        passIn.backImg,          cam.camBuffer,        cull,
                        passIn.sceneConfig,      passIn.meshInstances, passIn.positions,
                        passIn.normals,          passIn.uvs,           passIn.indices,
                        passIn.matrices,         passIn.materials,     passIn.samplers,
                        passIn.numValidSampler,  passIn.lighting,      passIn.lights,
                        passIn.drawGroupCount,   passIn.atmosSetting,  passIn.atmosphere,
                        passIn.shadowMapSetting, passIn.shadowmap,
                      })
                    .out();

  passOut = {
    .backImg = deferred.backImg,
  };
}
}
