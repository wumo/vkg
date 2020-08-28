#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/model/camera.hpp"
#include "vkg/render/model/vertex.hpp"
#include "comp_tlas_pass.hpp"
#include "vkg/render/pass/atmosphere/atmosphere_pass.hpp"

namespace vkg {
struct RayTracingPassIn {
  FrameGraphResource<Texture *> backImg;
  FrameGraphResource<Camera *> camera;
  FrameGraphResource<SceneConfig> sceneConfig;
  FrameGraphResource<BufferInfo> meshInstances;
  FrameGraphResource<uint32_t> meshInstancesCount;
  FrameGraphResource<BufferInfo> positions;
  FrameGraphResource<BufferInfo> normals;
  FrameGraphResource<BufferInfo> uvs;
  FrameGraphResource<BufferInfo> indices;
  FrameGraphResource<BufferInfo> primitives;
  FrameGraphResource<BufferInfo> matrices;
  FrameGraphResource<BufferInfo> materials;
  FrameGraphResource<std::span<vk::DescriptorImageInfo>> samplers;
  FrameGraphResource<uint32_t> numValidSampler;
  FrameGraphResource<BufferInfo> lighting;
  FrameGraphResource<BufferInfo> lights;

  FrameGraphResource<std::span<uint32_t>> countPerDrawGroup;

  FrameGraphResource<AtmosphereSetting> atmosSetting;
  AtmospherePassOut atmosphere;
};
struct RayTracingPassOut {
  FrameGraphResource<Texture *> backImg;
};
class RayTracingPass: public Pass<RayTracingPassIn, RayTracingPassOut> {
public:
  void setup(PassBuilder &builder) override;
  void compile(RenderContext &ctx, Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  struct RTSetDef: DescriptorSetDef {
    __accelerationStructure__(
      as, vkStage::eRaygenNV | vkStage::eClosestHitNV | vkStage::eMissNV);
    __image2D__(hdr, vkStage::eRaygenNV | vkStage::eMissNV);
    __image2D__(depth, vkStage::eRaygenNV | vkStage::eMissNV);

    __uniform__(
      camera, vkStage::eVertex | vkStage::eFragment | vkStage::eRaygenNV |
                vkStage::eClosestHitNV | vkStage::eMissNV);

    __buffer__(
      meshInstances, vkStage::eVertex | vkStage::eClosestHitNV | vkStage::eMissNV);

    __buffer__(primitives, vkStage::eClosestHitNV | vkStage::eMissNV);
    __buffer__(positions, vkStage::eClosestHitNV | vkStage::eMissNV);
    __buffer__(normals, vkStage::eClosestHitNV | vkStage::eMissNV);
    __buffer__(uvs, vkStage::eClosestHitNV | vkStage::eMissNV);
    __buffer__(indices, vkStage::eClosestHitNV | vkStage::eMissNV);

    __buffer__(materials, vkStage::eFragment | vkStage::eClosestHitNV | vkStage::eMissNV);
    __sampler2D__(
      textures, vkStage::eFragment | vkStage::eClosestHitNV | vkStage::eMissNV);

    __uniform__(lighting, vkStage::eFragment | vkStage::eClosestHitNV | vkStage::eMissNV);
    __buffer__(lights, vkStage::eFragment | vkStage::eClosestHitNV | vkStage::eMissNV);
  } rtSetDef;

  struct AtmosphereSetDef: DescriptorSetDef {
    __uniform__(atmosphere, vkStage::eFragment);
    __uniform__(sun, vkStage::eFragment);
    __sampler2D__(transmittance, vkStage::eFragment);
    __sampler3D__(scattering, vkStage::eFragment);
    __sampler2D__(irradiance, vkStage::eFragment);
  } atmosphereSetDef;

  struct PushConstant {
    uint32_t frame;
  } pushConstant{};
  struct RTPipeDef: PipelineLayoutDef {
    __push_constant__(
      constant, vkStage::eRaygenNV | vkStage::eClosestHitNV, PushConstant);
    __set__(rt, RTSetDef);
    __set__(atmosphere, AtmosphereSetDef);
  } rtPipeDef;

  struct FrameResource {
    vk::DescriptorSet set;

    ASDesc tlas;
  };

  std::vector<FrameResource> frames;

  CompTLASPassOut compTlasPassOut;
};
}
