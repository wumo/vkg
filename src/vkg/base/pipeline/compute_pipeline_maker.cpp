#include "compute_pipeline_maker.hpp"
namespace vkg {
ComputePipelineMaker::ComputePipelineMaker(Device &device): device(device) {}
auto ComputePipelineMaker::layout(vk::PipelineLayout layout) -> ComputePipelineMaker & {
  layout_ = layout;
  return *this;
}
auto ComputePipelineMaker::shader(Shader &&shader) -> ComputePipelineMaker & {
  shader_ = std::move(shader);
  return *this;
}
auto ComputePipelineMaker::createUnique(vk::PipelineCache pipelineCache)
  -> vk::UniquePipeline {
  shader_.make(device.vkDevice());
  auto sp = shader_.specializationInfo();
  vk::PipelineShaderStageCreateInfo stage{
    {}, vk::ShaderStageFlagBits::eCompute, shader_.shaderModule(), "main", &sp};

  vk::ComputePipelineCreateInfo pipelineInfo;
  pipelineInfo.stage = stage;
  pipelineInfo.layout = layout_;
  return device.vkDevice().createComputePipelineUnique(pipelineCache, pipelineInfo);
}
}
