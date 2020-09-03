#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/model/camera.hpp"
#include "vkg/render/model/vertex.hpp"
#include "vkg/render/pass/atmosphere/atmosphere_pass.hpp"
#include "vkg/render/pass/common/cam_frustum_pass.hpp"
#include "comp_tlas_pass.hpp"

namespace vkg {
struct TraceRaysPassIn {
  FrameGraphResource<Texture *> backImg;
  FrameGraphResource<BufferInfo> camBuffer;
  CompTLASPassOut compTlasPassOut;

  FrameGraphResource<SceneConfig> sceneConfig;
  FrameGraphResource<BufferInfo> meshInstances;
  FrameGraphResource<BufferInfo> positions;
  FrameGraphResource<BufferInfo> normals;
  FrameGraphResource<BufferInfo> uvs;
  FrameGraphResource<BufferInfo> indices;
  FrameGraphResource<BufferInfo> primitives;
  FrameGraphResource<BufferInfo> materials;
  FrameGraphResource<std::span<vk::DescriptorImageInfo>> samplers;
  FrameGraphResource<uint32_t> numValidSampler;
  FrameGraphResource<BufferInfo> lighting;
  FrameGraphResource<BufferInfo> lights;

  FrameGraphResource<AtmosphereSetting> atmosSetting;
  AtmospherePassOut atmosphere;
};
struct TraceRaysPassOut {
  FrameGraphResource<Texture *> backImg;
  FrameGraphResource<Texture *> depthImg;
};

class TraceRaysPass: public Pass<TraceRaysPassIn, TraceRaysPassOut> {
public:
  void setup(PassBuilder &builder) override;
  void compile(RenderContext &ctx, Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  bool refitTLAS{true};

  struct RTSetDef: DescriptorSetDef {
    __accelerationStructure__(
      as, vkStage::eRaygenNV | vkStage::eClosestHitNV | vkStage::eMissNV);
    __image2D__(hdr, vkStage::eRaygenNV | vkStage::eMissNV);
    __image2D__(depth, vkStage::eRaygenNV | vkStage::eMissNV);

    __buffer__(camera, vkStage::eRaygenNV | vkStage::eClosestHitNV | vkStage::eMissNV);

    __buffer__(
      meshInstances, vkStage::eClosestHitNV | vkStage::eMissNV | vkStage::eAnyHitNV);

    __buffer__(
      primitives, vkStage::eClosestHitNV | vkStage::eMissNV | vkStage::eAnyHitNV);
    __buffer__(positions, vkStage::eClosestHitNV | vkStage::eMissNV | vkStage::eAnyHitNV);
    __buffer__(normals, vkStage::eClosestHitNV | vkStage::eMissNV | vkStage::eAnyHitNV);
    __buffer__(uvs, vkStage::eClosestHitNV | vkStage::eMissNV | vkStage::eAnyHitNV);
    __buffer__(indices, vkStage::eClosestHitNV | vkStage::eMissNV | vkStage::eAnyHitNV);

    __buffer__(materials, vkStage::eClosestHitNV | vkStage::eMissNV | vkStage::eAnyHitNV);
    __sampler2D__(
      textures, vkStage::eClosestHitNV | vkStage::eMissNV | vkStage::eAnyHitNV);

    __uniform__(lighting, vkStage::eClosestHitNV | vkStage::eMissNV);
    __buffer__(lights, vkStage::eClosestHitNV | vkStage::eMissNV);
  } rtSetDef;

  struct AtmosphereSetDef: DescriptorSetDef {
    __uniform__(atmosphere, vkStage::eClosestHitNV | vkStage::eMissNV);
    __uniform__(sun, vkStage::eClosestHitNV | vkStage::eMissNV);
    __sampler2D__(transmittance, vkStage::eClosestHitNV | vkStage::eMissNV);
    __sampler3D__(scattering, vkStage::eClosestHitNV | vkStage::eMissNV);
    __sampler2D__(irradiance, vkStage::eClosestHitNV | vkStage::eMissNV);
  } atmosphereSetDef;

  struct PushConstant {
    uint32_t maxDepth;
    uint32_t nbSamples;
    uint32_t frame;
  } pushConstant{};
  struct RTPipeDef: PipelineLayoutDef {
    __push_constant__(
      constant,
      vkStage::eRaygenNV | vkStage::eClosestHitNV | vkStage::eMissNV | vkStage::eAnyHitNV,
      PushConstant);
    __set__(rt, RTSetDef);
    __set__(atmosphere, AtmosphereSetDef);
  } pipeDef;

  struct FrameResource {
    Texture *backImg;
    std::unique_ptr<Texture> depthImg;
    uint64_t lastNumValidSampler{0};

    vk::DescriptorSet rtSet, atmosphereSet;

    ASDesc tlas;
  };
  std::vector<FrameResource> frames;

  vk::UniqueDescriptorPool descriptorPool;
  vk::UniquePipeline pipe, atmosPipe;
  ShaderBindingTable sbt, atmosSbt;

  bool init{false};
};
}
