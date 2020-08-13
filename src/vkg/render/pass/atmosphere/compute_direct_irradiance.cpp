#include "atmosphere_model.hpp"
#include "atmosphere/direct_irradiance_comp.hpp"
namespace vkg {
auto AtmosphereModel::createDirectIrradianceSets() -> void {
  directIrradiancePipe =
    ComputePipelineMaker(device)
      .layout(directIrradiancePipeDef.layout())
      .shader(Shader{shader::atmosphere::direct_irradiance_comp_span, 8, 8, 1})
      .createUnique(nullptr);

  directIrradianceSet = directIrradianceSetDef.createSet(*descriptorPool);
  directIrradianceSetDef.atmosphere(atmosphereUBO_->buffer());
  directIrradianceSetDef.transmittance(*transmittanceTex_);
  directIrradianceSetDef.delta_irradiance(deltaIrradianceTex->imageView());
  directIrradianceSetDef.irradiance(irradianceTex_->imageView());
  directIrradianceSetDef.update(directIrradianceSet);
}
auto AtmosphereModel::recordDirectIrradianceCMD(vk::CommandBuffer cb, vk::Bool32 cumulate)
  -> void {
  image::transitTo(
    cb, *deltaIrradianceTex, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
    vk::PipelineStageFlagBits::eComputeShader);
  image::transitTo(
    cb, *irradianceTex_, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
    vk::PipelineStageFlagBits::eComputeShader);

  cb.bindPipeline(vk::PipelineBindPoint::eCompute, *directIrradiancePipe);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eCompute, directIrradiancePipeDef.layout(),
    directIrradiancePipeDef.set.set(), directIrradianceSet, nullptr);
  cb.pushConstants<vk::Bool32>(
    directIrradiancePipeDef.layout(), vk::ShaderStageFlagBits::eCompute, 0, cumulate);
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