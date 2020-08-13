#include "atmosphere_model.hpp"
#include "atmosphere/indirect_irradiance_comp.hpp"
namespace vkg {
auto AtmosphereModel::createIndirectIrradianceSets() -> void {
  indirectIrradiancePipe =
    ComputePipelineMaker(device)
      .layout(indirectIrradiancePipeDef.layout())
      .shader(Shader{shader::atmosphere::indirect_irradiance_comp_span, 8, 8, 1})
      .createUnique(nullptr);

  indirectIrradianceSet = indirectIrradianceSetDef.createSet(*descriptorPool);
  indirectIrradianceSetDef.atmosphere(atmosphereUBO_->buffer());
  indirectIrradianceSetDef.delta_rayleigh(*deltaRayleighScatteringTex);
  indirectIrradianceSetDef.delta_mie(*deltaMieScatteringTex);
  indirectIrradianceSetDef.multpli_scattering(*deltaRayleighScatteringTex);
  indirectIrradianceSetDef.delta_irradiance(deltaIrradianceTex->imageView());
  indirectIrradianceSetDef.irradiance(irradianceTex_->imageView());
  indirectIrradianceSetDef.update(indirectIrradianceSet);
}
auto AtmosphereModel::recordIndirectIrradianceCMD(
  vk::CommandBuffer cb, const glm::mat4 &luminance_from_radiance, int32_t scatteringOrder)
  -> void {
  image::transitTo(
    cb, *deltaIrradianceTex, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
    vk::PipelineStageFlagBits::eComputeShader);
  image::transitTo(
    cb, *irradianceTex_, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
    vk::PipelineStageFlagBits::eComputeShader);

  cb.bindPipeline(vk::PipelineBindPoint::eCompute, *indirectIrradiancePipe);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eCompute, indirectIrradiancePipeDef.layout(),
    indirectIrradiancePipeDef.set.set(), indirectIrradianceSet, nullptr);
  ScatteringOrderLFUConstant constant{luminance_from_radiance, scatteringOrder};
  cb.pushConstants<ScatteringOrderLFUConstant>(
    indirectIrradiancePipeDef.layout(), vk::ShaderStageFlagBits::eCompute, 0, constant);
  cb.pushConstants<int32_t>(
    indirectIrradiancePipeDef.layout(), vk::ShaderStageFlagBits::eCompute,
    sizeof(glm::mat4), scatteringOrder);
  auto dim = irradianceTex_->extent();
  cb.dispatch(dim.width / 8, dim.height / 8, 1);

  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader,
    {}, nullptr, nullptr,
    {deltaIrradianceTex->barrier(
       vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderRead,
       vk::PipelineStageFlagBits::eComputeShader),
     irradianceTex_->barrier(
       vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderRead,
       vk::PipelineStageFlagBits::eComputeShader)});
}
}