#include "atmosphere_model.hpp"
#include "atmosphere/multiple_scattering_comp.hpp"
namespace vkg {
auto AtmosphereModel::createMultipleScatteringSets() -> void {
  multipleScatteringPipe =
    ComputePipelineMaker(device)
      .layout(multipleScatteringPipeDef.layout())
      .shader(Shader{shader::atmosphere::multiple_scattering_comp_span, 8, 8, 1})
      .createUnique(nullptr);

  multipleScatteringSet = multipleScatteringSetDef.createSet(*descriptorPool);
  multipleScatteringSetDef.atmosphere(atmosphereUBO_->buffer());
  multipleScatteringSetDef.transmittance(*transmittanceTex_);
  multipleScatteringSetDef.scattering_density(*deltaScatteringDensityTex);
  multipleScatteringSetDef.delta_multiple_scattering(
    deltaRayleighScatteringTex->imageView());
  multipleScatteringSetDef.scattering(scatteringTex_->imageView());
  multipleScatteringSetDef.update(multipleScatteringSet);
}
auto AtmosphereModel::recordMultipleScatteringCMD(
  vk::CommandBuffer cb, const glm::mat4 &luminance_from_radiance) -> void {
  image::transitTo(
    cb, *deltaRayleighScatteringTex, vk::ImageLayout::eGeneral,
    vk::AccessFlagBits::eShaderWrite, vk::PipelineStageFlagBits::eComputeShader);
  image::transitTo(
    cb, *scatteringTex_, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
    vk::PipelineStageFlagBits::eComputeShader);

  cb.bindPipeline(vk::PipelineBindPoint::eCompute, *multipleScatteringPipe);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eCompute, multipleScatteringPipeDef.layout(),
    multipleScatteringPipeDef.set.set(), multipleScatteringSet, nullptr);
  cb.pushConstants<glm::mat4>(
    multipleScatteringPipeDef.layout(), vk::ShaderStageFlagBits::eCompute, 0,
    luminance_from_radiance);
  auto dim = scatteringTex_->extent();
  cb.dispatch(dim.width / 8, dim.height / 8, dim.depth / 1);

  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader,
    {}, nullptr, nullptr,
    {deltaRayleighScatteringTex->barrier(
       vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderRead,
       vk::PipelineStageFlagBits::eComputeShader),
     scatteringTex_->barrier(
       vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderRead,
       vk::PipelineStageFlagBits::eComputeShader)});
}
}