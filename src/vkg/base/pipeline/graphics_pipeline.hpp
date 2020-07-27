#pragma once
#include "vkg/base/vk_headers.hpp"
#include <vector>
#include <map>
#include <cstddef>
#include <initializer_list>
#include "pipeline.hpp"
#include "shaders.hpp"

namespace vkg {
class BlendColorAttachmentMaker;

struct VertexInputAttribute {
  uint32_t binding{}, location{};
  vk::Format format{};
  size_t offset = 0;
};

struct VertexInputBinding {
  uint32_t binding{};
  uint32_t stride{};
  vk::VertexInputRate inputRate{vk::VertexInputRate::eVertex};
  std::initializer_list<VertexInputAttribute> attributes;
};

struct VertexInputAutoAttribute {
  vk::Format format{};
  size_t offset = 0;
};

struct VertexInputAutoBinding {
  uint32_t stride{};
  vk::VertexInputRate inputRate{vk::VertexInputRate::eVertex};
  std::initializer_list<VertexInputAutoAttribute> attributes;
};

class GraphicsPipelineMaker: SpecializationMaker {
  friend class VertexInputDescriptionMaker;
  friend class AutoVertexInputDescriptionMaker;
  friend class BlendColorAttachmentMaker;

public:
  explicit GraphicsPipelineMaker(vk::Device device);
  auto vertexInput(std::initializer_list<VertexInputBinding> bindings)
    -> GraphicsPipelineMaker &;
  auto vertexInputAuto(std::initializer_list<VertexInputAutoBinding> bindings)
    -> GraphicsPipelineMaker &;
  auto vertexInputBinding(const vk::VertexInputBindingDescription &binding)
    -> GraphicsPipelineMaker &;
  auto vertexInputAttribute(const vk::VertexInputAttributeDescription &attribute)
    -> GraphicsPipelineMaker &;

  auto inputAssembly(vk::PrimitiveTopology topology, vk::Bool32 restartEnable = false)
    -> GraphicsPipelineMaker &;

  auto tesselation(uint32_t patchControlPoints) -> GraphicsPipelineMaker &;

  auto viewport(vk::Viewport value) -> GraphicsPipelineMaker &;
  auto scissor(vk::Rect2D value) -> GraphicsPipelineMaker &;

  auto depthClampEnable(vk::Bool32 value) -> GraphicsPipelineMaker &;
  auto rasterizerDiscardEnable(vk::Bool32 value) -> GraphicsPipelineMaker &;
  auto polygonMode(vk::PolygonMode value) -> GraphicsPipelineMaker &;
  auto cullMode(vk::CullModeFlags value) -> GraphicsPipelineMaker &;
  auto frontFace(vk::FrontFace value) -> GraphicsPipelineMaker &;
  auto depthBiasEnable(vk::Bool32 value) -> GraphicsPipelineMaker &;
  auto depthBiasConstantFactor(float value) -> GraphicsPipelineMaker &;
  auto depthBiasClamp(float value) -> GraphicsPipelineMaker &;
  auto depthBiasSlopeFactor(float value) -> GraphicsPipelineMaker &;
  auto lineWidth(float value) -> GraphicsPipelineMaker &;

  auto rasterizationSamples(vk::SampleCountFlagBits value) -> GraphicsPipelineMaker &;
  auto sampleShadingEnable(vk::Bool32 value) -> GraphicsPipelineMaker &;
  auto minSampleShading(float value) -> GraphicsPipelineMaker &;
  auto pSampleMask(const vk::SampleMask *value) -> GraphicsPipelineMaker &;
  auto alphaToCoverageEnable(vk::Bool32 value) -> GraphicsPipelineMaker &;
  auto alphaToOneEnable(vk::Bool32 value) -> GraphicsPipelineMaker &;

  auto depthTestEnable(vk::Bool32 value) -> GraphicsPipelineMaker &;
  auto depthWriteEnable(vk::Bool32 value) -> GraphicsPipelineMaker &;
  auto depthCompareOp(vk::CompareOp value) -> GraphicsPipelineMaker &;
  auto depthBoundsTestEnable(vk::Bool32 value) -> GraphicsPipelineMaker &;
  auto stencilTestEnable(vk::Bool32 value) -> GraphicsPipelineMaker &;
  auto front(vk::StencilOpState value) -> GraphicsPipelineMaker &;
  auto back(vk::StencilOpState value) -> GraphicsPipelineMaker &;
  auto minDepthBounds(float value) -> GraphicsPipelineMaker &;
  auto maxDepthBounds(float value) -> GraphicsPipelineMaker &;

  auto logicOpEnable(vk::Bool32 value) -> GraphicsPipelineMaker &;
  auto logicOp(vk::LogicOp value) -> GraphicsPipelineMaker &;
  auto blendConstants(float r, float g, float b, float a) -> GraphicsPipelineMaker &;
  auto blendColorAttachment(vk::Bool32 enable) -> BlendColorAttachmentMaker;

  auto dynamicState(vk::DynamicState value) -> GraphicsPipelineMaker &;

  auto layout(vk::PipelineLayout layout) -> GraphicsPipelineMaker &;
  auto renderPass(vk::RenderPass renderPass) -> GraphicsPipelineMaker &;
  auto subpass(uint32_t subpass) -> GraphicsPipelineMaker &;

  auto shader(vk::ShaderStageFlagBits shaderStage, Shader &&shader)
    -> GraphicsPipelineMaker &;

  auto clearShader(vk::ShaderStageFlagBits shaderStage) -> GraphicsPipelineMaker &;

  auto createUnique(vk::PipelineCache pipelineCache = {}) -> vk::UniquePipeline;

private:
  vk::Device device;

  std::map<vk::ShaderStageFlagBits, Shader> shaders;
  std::vector<vk::VertexInputBindingDescription> _vertexBindings;
  std::vector<vk::VertexInputAttributeDescription> _vertexAttributes;
  vk::PipelineInputAssemblyStateCreateInfo _inputAssemblyState;
  vk::PipelineTessellationStateCreateInfo _tesselationState;
  std::vector<vk::Viewport> _viewports;
  std::vector<vk::Rect2D> _scissors;
  vk::PipelineRasterizationStateCreateInfo _rasterizationState;
  vk::PipelineMultisampleStateCreateInfo _multisampleState;
  vk::PipelineDepthStencilStateCreateInfo _depthStencilState;
  vk::PipelineColorBlendStateCreateInfo _colorBlendState;
  std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
  std::vector<vk::DynamicState> _dynamicState;
  uint32_t _subpass = 0;
  vk::PipelineLayout _layout;
  vk::RenderPass _renderPass;
};

class BlendColorAttachmentMaker {
public:
  BlendColorAttachmentMaker(GraphicsPipelineMaker &maker, uint32_t index);
  auto enable(vk::Bool32 value) -> BlendColorAttachmentMaker &;
  auto srcColorBlendFactor(vk::BlendFactor value) -> BlendColorAttachmentMaker &;
  auto dstColorBlendFactor(vk::BlendFactor value) -> BlendColorAttachmentMaker &;
  auto colorBlendOp(vk::BlendOp value) -> BlendColorAttachmentMaker &;
  auto srcAlphaBlendFactor(vk::BlendFactor value) -> BlendColorAttachmentMaker &;
  auto dstAlphaBlendFactor(vk::BlendFactor value) -> BlendColorAttachmentMaker &;
  auto alphaBlendOp(vk::BlendOp value) -> BlendColorAttachmentMaker &;
  auto colorWriteMask(vk::ColorComponentFlags value) -> BlendColorAttachmentMaker &;

private:
  GraphicsPipelineMaker &maker;
  uint32_t index;
};
}
