#include "atmosphere_model.hpp"
#include "atmosphere/transmittance_comp.hpp"
namespace vkg {
auto AtmosphereModel::createTransmittanceSets() -> void {
  transmittancePipe =
    ComputePipelineMaker(device)
      .layout(transmittancePipeDef.layout())
      .shader(Shader{shader::atmosphere::transmittance_comp_span, 8, 8, 1})
      .createUnique(nullptr);
  transmittanceSet = transmittanceSetDef.createSet(*descriptorPool);
  transmittanceSetDef.atmosphere(atmosphereUBO_->buffer());
  transmittanceSetDef.transmittance(transmittanceTex_->imageView());
  transmittanceSetDef.update(transmittanceSet);
}
auto AtmosphereModel::recordTransmittanceCMD(vk::CommandBuffer cb) -> void {
  image::transitTo(
    cb, *transmittanceTex_, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
    vk::PipelineStageFlagBits::eComputeShader);
  cb.bindPipeline(vk::PipelineBindPoint::eCompute, *transmittancePipe);
  cb.bindDescriptorSets(
    vk::PipelineBindPoint::eCompute, transmittancePipeDef.layout(),
    transmittancePipeDef.set.set(), transmittanceSet, nullptr);

  auto dim = transmittanceTex_->extent();
  cb.dispatch(dim.width / 8, dim.height / 8, 1);

  cb.pipelineBarrier(
    vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader,
    {}, nullptr, nullptr,
    transmittanceTex_->barrier(
      vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderRead,
      vk::PipelineStageFlagBits::eComputeShader));
}
}