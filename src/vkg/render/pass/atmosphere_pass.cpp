#include "atmosphere_pass.hpp"
namespace vkg {

auto AtmospherePass::setup(PassBuilder &builder, const AtmospherePassIn &inputs)
  -> AtmospherePassOut {
  passIn = inputs;
  builder.read(passIn.atmosphere);
  passOut.version = builder.create<uint64_t>("atmosphereVersion");
  passOut.atmosphere = builder.create<vk::Buffer>("atmosphere");
  passOut.sun = builder.create<vk::Buffer>("sun");
  passOut.transmittance = builder.create<Texture *>("transmittance");
  passOut.scattering = builder.create<Texture *>("scattering");
  passOut.irradiance = builder.create<Texture *>("irradiance");
  return passOut;
}
void AtmospherePass::compile(RenderContext &ctx, Resources &resources) {}
void AtmospherePass::execute(RenderContext &ctx, Resources &resources) {}
}