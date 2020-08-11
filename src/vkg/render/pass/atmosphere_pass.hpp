#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/model/atmosphere.hpp"

namespace vkg {
struct AtmospherePassIn {
  FrameGraphResource<Atmosphere> atmosphere;
};
struct AtmospherePassOut {
  FrameGraphResource<uint64_t> version;
  FrameGraphResource<vk::Buffer> atmosphere;
  FrameGraphResource<vk::Buffer> sun;
  FrameGraphResource<Texture *> transmittance;
  FrameGraphResource<Texture *> scattering;
  FrameGraphResource<Texture *> irradiance;
};

class AtmospherePass: public Pass<AtmospherePassIn, AtmospherePassOut> {
public:
  auto setup(PassBuilder &builder, const AtmospherePassIn &inputs)
    -> AtmospherePassOut override;
  void compile(Resources &resources) override;
  void execute(RenderContext &ctx, Resources &resources) override;

private:
  AtmospherePassIn passIn;
  AtmospherePassOut passOut;
};
}
