#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/model/camera.hpp"
namespace vkg {
struct DeferredPassIn {
  FrameGraphResource<vk::Extent2D> swapchainExtent;
  FrameGraphResource<Camera *> camera;
  FrameGraphResource<vk::Buffer> cameraBuffer;
  FrameGraphResource<vk::Buffer> meshInstances;
  FrameGraphResource<uint32_t> meshInstancesCount;
  FrameGraphResource<uint32_t> maxNumMeshInstances;
  FrameGraphResource<vk::Buffer> primitives;
  FrameGraphResource<vk::Buffer> matrices;
  FrameGraphResource<vk::Buffer> materials;
  FrameGraphResource<std::vector<vk::DescriptorImageInfo> *> textures;
  FrameGraphResource<vk::Buffer> lighting;
  FrameGraphResource<vk::Buffer> lights;

  FrameGraphResource<std::vector<uint32_t>> drawGroupCount;
};
struct DeferredPassOut {
  FrameGraphResource<vk::Buffer> camFrustum;
  FrameGraphResource<vk::Buffer> drawCMDBuffer;
  FrameGraphResource<vk::Buffer> drawCMDCountBuffer;
};
class DeferredPass: public Pass<DeferredPassIn, DeferredPassOut> {
public:
  auto setup(PassBuilder &builder, const DeferredPassIn &inputs)
    -> DeferredPassOut override;
  void compile(Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  DeferredPassIn passIn;
  DeferredPassOut passOut;

  struct SceneSetDef: DescriptorSetDef {
    __uniform__(cam, vkStage::eVertex | vkStage::eFragment);
    __buffer__(meshInstances, vkStage::eVertex);
    __buffer__(primitives, vkStage::eVertex);
    __buffer__(matrices, vkStage::eVertex);
    __buffer__(materials, vkStage::eFragment);
    __sampler2D__(textures, vkStage::eFragment);

    __uniform__(lighting, vkStage::eFragment);
    __buffer__(lights, vkStage::eFragment);
  } sceneSetDef;

  struct GBufferSetDef: DescriptorSetDef {
    __input__(position, vkStage::eFragment);
    __input__(normal, vkStage::eFragment);
    __input__(diffuse, vkStage::eFragment);
    __input__(specular, vkStage::eFragment);
    __input__(emissive, vkStage::eFragment);
    __input__(depth, vkStage::eFragment);
  } gbufferSetDef;

  struct CSMSetDef: DescriptorSetDef {
    __uniform__(setting, vkStage::eFragment);
    __uniform__(cascades, vkStage::eFragment);
    __sampler2DArray__(shadowMaps, vkStage::eFragment);
  } csmSetDef;

  struct AtmosphereSetDef: DescriptorSetDef {
    __uniform__(atmosphere, vkStage::eFragment);
    __uniform__(sun, vkStage::eFragment);
    __sampler3D__(transmittance, vkStage::eFragment);
    __sampler3D__(scattering, vkStage::eFragment);
    __sampler2D__(irradiance, vkStage::eFragment);
  } atmosphereSetDef;

  struct DeferredPipeDef: PipelineLayoutDef {
    __set__(basic, SceneSetDef);
    __set__(gbuffer, GBufferSetDef);
    __set__(atmosphere, AtmosphereSetDef);
    __set__(shadowMap, CSMSetDef);
  } deferredPipeDef;

  std::unique_ptr<Buffer> camFrustum;
  std::unique_ptr<Buffer> drawCMD;
  std::unique_ptr<Buffer> drawCMDCount;

  vk::SampleCountFlagBits sampleCount{vk::SampleCountFlagBits ::e1};
  bool enableSampleShading{true};
  float minSampleShading{1};
  bool wireframe_{false};
  float lineWidth_{1.f};

  bool init{false};

  std::unique_ptr<Texture> colorAtt, depthAtt, positionAtt, normalAtt, diffuseAtt,
    specularAtt, emissiveAtt;

  vk::UniqueDescriptorPool descriptorPool;
  vk::DescriptorSet sceneSet, gbufferSet, shadowMapSet, atmosphereSet;

  uint32_t gbufferPass{}, lightingPass{}, transPass{}, resolvePass{};
  vk::UniqueRenderPass renderPass;
  vk::UniquePipeline gbTriPipe, gbLinePipe, gbWireFramePipe, lightingPipe, transTriPipe,
    transLinePipe;
  std::vector<vk::UniqueFramebuffer> framebuffers;
};
}
