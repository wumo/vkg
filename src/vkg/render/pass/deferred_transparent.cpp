#include "deferred.hpp"
#include "deferred/deferred_vert.hpp"
#include "deferred/transparent_frag.hpp"

namespace vkg {
auto DeferredPass::createTransparentPass(Device &device, SceneConfig sceneConfig)
  -> void {
  GraphicsPipelineMaker maker(device.vkDevice());

  maker.layout(deferredPipeDef.layout())
    .renderPass(*renderPass)
    .subpass(transPass)
    .vertexInputAuto(
      {{.stride = sizeof(Vertex::Position),
        .attributes = {{vk::Format::eR32G32B32Sfloat}}},
       {.stride = sizeof(Vertex::Normal), .attributes = {{vk::Format::eR32G32B32Sfloat}}},
       {.stride = sizeof(Vertex::UV), .attributes = {{vk::Format::eR32G32Sfloat}}}})
    .inputAssembly(vk::PrimitiveTopology::eTriangleList)
    .polygonMode(vk::PolygonMode::eFill)
    .cullMode(vk::CullModeFlagBits::eNone)
    .frontFace(vk::FrontFace::eCounterClockwise)
    .depthTestEnable(true)
    .depthWriteEnable(false)
    .depthCompareOp(vk::CompareOp::eLessOrEqual)
    .viewport({})
    .scissor({})
    .dynamicState(vk::DynamicState::eViewport)
    .dynamicState(vk::DynamicState::eScissor)
    .dynamicState(vk::DynamicState::eLineWidth);

  maker.blendColorAttachment(true)
    .srcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
    .dstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
    .colorBlendOp(vk::BlendOp::eAdd)
    .srcAlphaBlendFactor(vk::BlendFactor::eOne)
    .dstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
    .alphaBlendOp(vk::BlendOp::eAdd)
    .colorWriteMask(
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

  maker
    .shader(
      vk::ShaderStageFlagBits::eVertex,
      Shader{
        shader::deferred::deferred_vert_span, sceneConfig.maxNumTextures,
        sceneConfig.maxNumLights})
    .shader(
      vk::ShaderStageFlagBits::eFragment,
      Shader{shader::deferred::transparent_frag_span, sceneConfig.maxNumTextures});
  transTriPipe = maker.createUnique();
  device.name(*transTriPipe, "transparent triangle pipeline");

  maker.inputAssembly(vk::PrimitiveTopology::eLineList);
  transLinePipe = maker.createUnique();
  device.name(*transLinePipe, "transparent line pipeline");
}
}