#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/render/model/atmosphere.hpp"
#include "atmosphere_model.hpp"

namespace vkg {
struct AtmospherePassIn {
  FrameGraphResource<AtmosphereSetting> atmosphere;
};
struct AtmospherePassOut {
  FrameGraphResource<uint64_t> version;
  FrameGraphResource<BufferInfo> atmosphere;
  FrameGraphResource<BufferInfo> sun;
  FrameGraphResource<Texture *> transmittance;
  FrameGraphResource<Texture *> scattering;
  FrameGraphResource<Texture *> irradiance;
};

class AtmospherePass: public Pass<AtmospherePassIn, AtmospherePassOut> {
public:
  void setup(PassBuilder &builder) override;
  void compile(RenderContext &ctx, Resources &resources) override;

private:
  uint64_t version{0};
  std::unique_ptr<AtmosphereModel> model_;
};
}
