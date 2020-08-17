#include "atmosphere_model.hpp"
#include "atmosphere/single_scattering_comp.hpp"
namespace vkg {
auto AtmosphereModel::createSingleScatteringSets() -> void {
  singleScatteringPipe =
    ComputePipelineMaker(device)
      .layout(singleScatteringPipeDef.layout())
      .shader(Shader{shader::atmosphere::single_scattering_comp_span, 8, 8, 1})
      .createUnique(nullptr);

  singleScatteringSet = singleScatteringSetDef.createSet(*descriptorPool);
  singleScatteringSetDef.atmosphere(atmosphereUBO_->bufferInfo());
  singleScatteringSetDef.transmittance(*transmittanceTex_);
  singleScatteringSetDef.delta_rayleigh(deltaRayleighScatteringTex->imageView());
  singleScatteringSetDef.delta_mie(deltaMieScatteringTex->imageView());
  singleScatteringSetDef.scattering(scatteringTex_->imageView());
  singleScatteringSetDef.update(singleScatteringSet);
}
auto AtmosphereModel::recordSingleScatteringCMD(
  vk::CommandBuffer cb, const glm::mat4 &luminance_from_radiance, vk::Bool32 cumulate)
  -> void {
  image::transitTo(
    cb, *deltaRayleighScatteringTex, vk::ImageLayout::eGeneral,
    vk::AccessFlagBits::eShaderWrite, vk::PipelineStageFlagBits::eComputeShader);
  image::transitTo(
    cb, *deltaMieScatteringTex, vk::ImageLayout::eGeneral,
    vk::AccessFlagBits::eShaderWrite, vk::PipelineStageFlagBits::eComputeShader);
  image::transitTo(
    cb, *scatteringTex_, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
    vk::PipelineStageFlagBits::eComputeShader);

  cb.bindPipeline(vk::PipelineBindPoint::eCompute, *singleScatteringPipe);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eCompute, singleScatteringPipeDef.layout(),
    singleScatteringPipeDef.set.set(), singleScatteringSet, nullptr);
  CumulateLFUConstant constant{luminance_from_radiance, cumulate};
  cb.pushConstants<CumulateLFUConstant>(
    singleScatteringPipeDef.layout(), vk::ShaderStageFlagBits::eCompute, 0, constant);
  auto dim = scatteringTex_->extent();
  cb.dispatch(dim.width / 8, dim.height / 8, dim.depth / 1);

  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader,
    {}, nullptr, nullptr,
    {deltaRayleighScatteringTex->barrier(
       vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderRead,
       vk::PipelineStageFlagBits::eComputeShader),
     deltaMieScatteringTex->barrier(
       vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderRead,
       vk::PipelineStageFlagBits::eComputeShader),
     scatteringTex_->barrier(
       vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderRead,
       vk::PipelineStageFlagBits::eComputeShader)});
}

}