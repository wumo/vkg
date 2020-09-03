#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/model/camera.hpp"
#include "vkg/render/pass/atmosphere/atmosphere_pass.hpp"
#include "vkg/render/pass/common/cam_frustum_pass.hpp"
#include "comp_tlas_pass.hpp"
#include "trace_rays_pass.hpp"
#include "forward_pass.hpp"

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
};
}
