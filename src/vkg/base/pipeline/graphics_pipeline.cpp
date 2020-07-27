#include "graphics_pipeline.hpp"
#include "shaders.hpp"

namespace vkg {
GraphicsPipelineMaker::GraphicsPipelineMaker(vk::Device device): device(device) {}

auto GraphicsPipelineMaker::vertexInput(
  std::initializer_list<VertexInputBinding> bindings) -> GraphicsPipelineMaker & {
  for(auto &binding: bindings) {
    _vertexBindings.emplace_back(binding.binding, binding.stride, binding.inputRate);
    for(auto &attr: binding.attributes)
      _vertexAttributes.emplace_back(
        attr.location, attr.binding, attr.format, attr.offset);
  }
  return *this;
}

auto GraphicsPipelineMaker::vertexInputAuto(
  std::initializer_list<VertexInputAutoBinding> bindings) -> GraphicsPipelineMaker & {
  uint32_t bindingIdx = 0, location = 0;
  for(auto &binding: bindings) {
    auto _bindingIdx = bindingIdx++;
    _vertexBindings.emplace_back(_bindingIdx, binding.stride, binding.inputRate);
    for(auto &attr: binding.attributes)
      _vertexAttributes.emplace_back(location++, _bindingIdx, attr.format, attr.offset);
  }
  return *this;
}

auto GraphicsPipelineMaker::vertexInputBinding(
  const vk::VertexInputBindingDescription &binding) -> GraphicsPipelineMaker & {
  return *this;
}

auto GraphicsPipelineMaker::vertexInputAttribute(
  const vk::VertexInputAttributeDescription &attribute) -> GraphicsPipelineMaker & {
  return *this;
}

auto GraphicsPipelineMaker::inputAssembly(
  vk::PrimitiveTopology topology, vk::Bool32 restartEnable) -> GraphicsPipelineMaker & {
  _inputAssemblyState.topology = topology;
  _inputAssemblyState.primitiveRestartEnable = restartEnable;
  return *this;
}

auto GraphicsPipelineMaker::tesselation(uint32_t patchControlPoints)
  -> GraphicsPipelineMaker & {
  _tesselationState.patchControlPoints = patchControlPoints;
  return *this;
}

auto GraphicsPipelineMaker::viewport(vk::Viewport value) -> GraphicsPipelineMaker & {
  _viewports.push_back(value);
  return *this;
}
auto GraphicsPipelineMaker::scissor(vk::Rect2D value) -> GraphicsPipelineMaker & {
  _scissors.push_back(value);
  return *this;
}

auto GraphicsPipelineMaker::depthClampEnable(vk::Bool32 value)
  -> GraphicsPipelineMaker & {
  _rasterizationState.depthClampEnable = value;
  return *this;
}
auto GraphicsPipelineMaker::rasterizerDiscardEnable(vk::Bool32 value)
  -> GraphicsPipelineMaker & {
  _rasterizationState.rasterizerDiscardEnable = value;
  return *this;
}
auto GraphicsPipelineMaker::polygonMode(vk::PolygonMode value)
  -> GraphicsPipelineMaker & {
  _rasterizationState.polygonMode = value;
  return *this;
}
auto GraphicsPipelineMaker::cullMode(vk::CullModeFlags value) -> GraphicsPipelineMaker & {
  _rasterizationState.cullMode = value;
  return *this;
}
auto GraphicsPipelineMaker::frontFace(vk::FrontFace value) -> GraphicsPipelineMaker & {
  _rasterizationState.frontFace = value;
  return *this;
}
auto GraphicsPipelineMaker::depthBiasEnable(vk::Bool32 value) -> GraphicsPipelineMaker & {
  _rasterizationState.depthBiasEnable = value;
  return *this;
}
auto GraphicsPipelineMaker::depthBiasConstantFactor(float value)
  -> GraphicsPipelineMaker & {
  _rasterizationState.depthBiasConstantFactor = value;
  return *this;
}
auto GraphicsPipelineMaker::depthBiasClamp(float value) -> GraphicsPipelineMaker & {
  _rasterizationState.depthBiasClamp = value;
  return *this;
}
auto GraphicsPipelineMaker::depthBiasSlopeFactor(float value) -> GraphicsPipelineMaker & {
  _rasterizationState.depthBiasSlopeFactor = value;
  return *this;
}
auto GraphicsPipelineMaker::lineWidth(float value) -> GraphicsPipelineMaker & {
  _rasterizationState.lineWidth = value;
  return *this;
}

auto GraphicsPipelineMaker::rasterizationSamples(vk::SampleCountFlagBits value)
  -> GraphicsPipelineMaker & {
  _multisampleState.rasterizationSamples = value;
  return *this;
}
auto GraphicsPipelineMaker::sampleShadingEnable(vk::Bool32 value)
  -> GraphicsPipelineMaker & {
  _multisampleState.sampleShadingEnable = value;
  return *this;
}
auto GraphicsPipelineMaker::minSampleShading(float value) -> GraphicsPipelineMaker & {
  _multisampleState.minSampleShading = value;
  return *this;
}
auto GraphicsPipelineMaker::pSampleMask(const vk::SampleMask *value)
  -> GraphicsPipelineMaker & {
  _multisampleState.pSampleMask = value;
  return *this;
}
auto GraphicsPipelineMaker::alphaToCoverageEnable(vk::Bool32 value)
  -> GraphicsPipelineMaker & {
  _multisampleState.alphaToCoverageEnable = value;
  return *this;
}
auto GraphicsPipelineMaker::alphaToOneEnable(vk::Bool32 value)
  -> GraphicsPipelineMaker & {
  _multisampleState.alphaToOneEnable = value;
  return *this;
}

auto GraphicsPipelineMaker::depthTestEnable(vk::Bool32 value) -> GraphicsPipelineMaker & {
  _depthStencilState.depthTestEnable = value;
  return *this;
}
auto GraphicsPipelineMaker::depthWriteEnable(vk::Bool32 value)
  -> GraphicsPipelineMaker & {
  _depthStencilState.depthWriteEnable = value;
  return *this;
}
auto GraphicsPipelineMaker::depthCompareOp(vk::CompareOp value)
  -> GraphicsPipelineMaker & {
  _depthStencilState.depthCompareOp = value;
  return *this;
}
auto GraphicsPipelineMaker::depthBoundsTestEnable(vk::Bool32 value)
  -> GraphicsPipelineMaker & {
  _depthStencilState.depthBoundsTestEnable = value;
  return *this;
}
auto GraphicsPipelineMaker::stencilTestEnable(vk::Bool32 value)
  -> GraphicsPipelineMaker & {
  _depthStencilState.stencilTestEnable = value;
  return *this;
}
auto GraphicsPipelineMaker::front(vk::StencilOpState value) -> GraphicsPipelineMaker & {
  _depthStencilState.front = value;
  return *this;
}
auto GraphicsPipelineMaker::back(vk::StencilOpState value) -> GraphicsPipelineMaker & {
  _depthStencilState.back = value;
  return *this;
}
auto GraphicsPipelineMaker::minDepthBounds(float value) -> GraphicsPipelineMaker & {
  _depthStencilState.minDepthBounds = value;
  return *this;
}
auto GraphicsPipelineMaker::maxDepthBounds(float value) -> GraphicsPipelineMaker & {
  _depthStencilState.maxDepthBounds = value;
  return *this;
}
auto GraphicsPipelineMaker::logicOpEnable(vk::Bool32 value) -> GraphicsPipelineMaker & {
  _colorBlendState.logicOpEnable = value;
  return *this;
}
auto GraphicsPipelineMaker::logicOp(vk::LogicOp value) -> GraphicsPipelineMaker & {
  _colorBlendState.logicOp = value;
  return *this;
}
auto GraphicsPipelineMaker::blendConstants(float r, float g, float b, float a)
  -> GraphicsPipelineMaker & {
  auto &bc = _colorBlendState.blendConstants;
  bc[0] = r;
  bc[1] = g;
  bc[2] = b;
  bc[3] = a;
  return *this;
}
auto GraphicsPipelineMaker::blendColorAttachment(vk::Bool32 enable)
  -> BlendColorAttachmentMaker {
  colorBlendAttachments.emplace_back();
  auto &blend = colorBlendAttachments.back();
  blend.blendEnable = enable;
  blend.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
  blend.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
  blend.colorBlendOp = vk::BlendOp::eAdd;
  blend.srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha;
  blend.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
  blend.alphaBlendOp = vk::BlendOp::eAdd;
  using flag = vk::ColorComponentFlagBits;
  blend.colorWriteMask = flag::eR | flag::eG | flag::eB | flag::eA;
  return BlendColorAttachmentMaker{*this, uint32_t(colorBlendAttachments.size() - 1)};
}
BlendColorAttachmentMaker::BlendColorAttachmentMaker(
  GraphicsPipelineMaker &maker, uint32_t index)
  : maker{maker}, index{index} {}
auto BlendColorAttachmentMaker::enable(vk::Bool32 value) -> BlendColorAttachmentMaker & {
  maker.colorBlendAttachments[index].blendEnable = value;
  return *this;
}
auto BlendColorAttachmentMaker::srcColorBlendFactor(vk::BlendFactor value)
  -> BlendColorAttachmentMaker & {
  maker.colorBlendAttachments[index].srcColorBlendFactor = value;
  return *this;
}
auto BlendColorAttachmentMaker::dstColorBlendFactor(vk::BlendFactor value)
  -> BlendColorAttachmentMaker & {
  maker.colorBlendAttachments[index].dstColorBlendFactor = value;
  return *this;
}
auto BlendColorAttachmentMaker::colorBlendOp(vk::BlendOp value)
  -> BlendColorAttachmentMaker & {
  maker.colorBlendAttachments[index].colorBlendOp = value;
  return *this;
}
auto BlendColorAttachmentMaker::srcAlphaBlendFactor(vk::BlendFactor value)
  -> BlendColorAttachmentMaker & {
  maker.colorBlendAttachments[index].srcAlphaBlendFactor = value;
  return *this;
}
auto BlendColorAttachmentMaker::dstAlphaBlendFactor(vk::BlendFactor value)
  -> BlendColorAttachmentMaker & {
  maker.colorBlendAttachments[index].dstAlphaBlendFactor = value;
  return *this;
}
auto BlendColorAttachmentMaker::alphaBlendOp(vk::BlendOp value)
  -> BlendColorAttachmentMaker & {
  maker.colorBlendAttachments[index].alphaBlendOp = value;
  return *this;
}
auto BlendColorAttachmentMaker::colorWriteMask(vk::ColorComponentFlags value)
  -> BlendColorAttachmentMaker & {
  maker.colorBlendAttachments[index].colorWriteMask = value;
  return *this;
}

auto GraphicsPipelineMaker::dynamicState(vk::DynamicState value)
  -> GraphicsPipelineMaker & {
  _dynamicState.push_back(value);
  return *this;
}

auto GraphicsPipelineMaker::layout(vk::PipelineLayout layout) -> GraphicsPipelineMaker & {
  _layout = layout;
  return *this;
}
auto GraphicsPipelineMaker::renderPass(vk::RenderPass renderPass)
  -> GraphicsPipelineMaker & {
  _renderPass = renderPass;
  return *this;
}
auto GraphicsPipelineMaker::subpass(uint32_t subpass) -> GraphicsPipelineMaker & {
  _subpass = subpass;
  return *this;
}
auto GraphicsPipelineMaker::shader(vk::ShaderStageFlagBits shaderStage, Shader &&shader)
  -> GraphicsPipelineMaker & {
  shaders.insert({shaderStage, std::move(shader)});
  return *this;
}
auto GraphicsPipelineMaker::clearShader(vk::ShaderStageFlagBits shaderStage)
  -> GraphicsPipelineMaker & {
  shaders.erase(shaderStage);
  return *this;
}

auto GraphicsPipelineMaker::createUnique(vk::PipelineCache pipelineCache)
  -> vk::UniquePipeline {
  _colorBlendState.attachmentCount = uint32_t(colorBlendAttachments.size());
  _colorBlendState.pAttachments = colorBlendAttachments.data();

  std::vector<vk::PipelineShaderStageCreateInfo> stages;
  stages.reserve(shaders.size());
  std::vector<vk::SpecializationInfo> specializations;
  specializations.reserve(shaders.size());
  for(auto &[stage, shader]: shaders) {
    const vk::SpecializationInfo *pSpecializationInfo{nullptr};
    specializations.push_back(shader.specializationInfo());
    pSpecializationInfo = &specializations.back();
    shader.make(device);
    stages.push_back(vk::PipelineShaderStageCreateInfo{
      {}, stage, shader.shaderModule(), "main", pSpecializationInfo});
  }

  vk::PipelineVertexInputStateCreateInfo vertexInputState{
    {},
    uint32_t(_vertexBindings.size()),
    _vertexBindings.data(),
    uint32_t(_vertexAttributes.size()),
    _vertexAttributes.data()};

  vk::PipelineViewportStateCreateInfo viewportState{
    {},
    uint32_t(_viewports.size()),
    _viewports.data(),
    uint32_t(_scissors.size()),
    _scissors.data()};

  vk::PipelineDynamicStateCreateInfo dynamicState{
    {}, uint32_t(_dynamicState.size()), _dynamicState.data()};

  vk::GraphicsPipelineCreateInfo pipelineInfo{
    {},
    uint32_t(shaders.size()),
    stages.data(),
    &vertexInputState,
    &_inputAssemblyState,
    &_tesselationState,
    &viewportState,
    &_rasterizationState,
    &_multisampleState,
    &_depthStencilState,
    &_colorBlendState,
    &dynamicState,
    _layout,
    _renderPass,
    _subpass};

  return device.createGraphicsPipelineUnique(pipelineCache, pipelineInfo);
}

}