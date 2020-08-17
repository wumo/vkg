#include "atmosphere_model.hpp"
#include "atmosphere/scattering_density_comp.hpp"
namespace vkg {
auto AtmosphereModel::createScatteringDensitySets() -> void {
  scatteringDensityPipe =
    ComputePipelineMaker(device)
      .layout(scatteringDensityPipeDef.layout())
      .shader(Shader{shader::atmosphere::scattering_density_comp_span, 8, 8, 1})
      .createUnique(nullptr);

  scatteringDensitySet = scatteringDensitySetDef.createSet(*descriptorPool);
  scatteringDensitySetDef.atmosphere(atmosphereUBO_->bufferInfo());
  scatteringDensitySetDef.transmittance(*transmittanceTex_);
  scatteringDensitySetDef.delta_rayleigh(*deltaRayleighScatteringTex);
  scatteringDensitySetDef.delta_mie(*deltaMieScatteringTex);
  scatteringDensitySetDef.multpli_scattering(*deltaRayleighScatteringTex);
  scatteringDensitySetDef.irradiance(*deltaIrradianceTex);
  scatteringDensitySetDef.scattering_density(deltaScatteringDensityTex->imageView());
  scatteringDensitySetDef.update(scatteringDensitySet);
}
auto AtmosphereModel::recordScatteringDensityCMD(
  vk::CommandBuffer cb, int32_t scatteringOrder) -> void {
  image::transitTo(
    cb, *deltaScatteringDensityTex, vk::ImageLayout::eGeneral,
    vk::AccessFlagBits::eShaderWrite, vk::PipelineStageFlagBits::eComputeShader);

  cb.bindPipeline(vk::PipelineBindPoint::eCompute, *scatteringDensityPipe);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eCompute, scatteringDensityPipeDef.layout(),
    scatteringDensityPipeDef.set.set(), scatteringDensitySet, nullptr);
  cb.pushConstants<int32_t>(
    scatteringDensityPipeDef.layout(), vk::ShaderStageFlagBits::eCompute, 0,
    scatteringOrder);
  auto dim = deltaScatteringDensityTex->extent();
  cb.dispatch(dim.width / 8, dim.height / 8, dim.depth / 1);

  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader,
    {}, nullptr, nullptr,
    deltaScatteringDensityTex->barrier(
      vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderRead,
      vk::PipelineStageFlagBits::eComputeShader));
}
}