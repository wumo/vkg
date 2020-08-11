#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/math/glm_common.hpp"

namespace vkg {
struct AtmospherePassIn {
  FrameGraphResource<float> exposure;
  FrameGraphResource<float> intensity;
  FrameGraphResource<glm::vec3> sunDirection;
  FrameGraphResource<glm::vec3> earthCenter;
};
struct AtmospherePassOut {
  FrameGraphResource<uint64_t> version;
  FrameGraphResource<vk::Buffer> atmosphere;
  FrameGraphResource<vk::Buffer> sun;
  FrameGraphResource<Texture *> transmittance;
  FrameGraphResource<Texture *> scattering;
  FrameGraphResource<Texture *> irradiance;
};

class Atmosphere {};
}
