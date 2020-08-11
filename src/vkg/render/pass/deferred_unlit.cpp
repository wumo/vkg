#include "deferred.hpp"
#include "deferred/deferred_vert.hpp"
#include "deferred/unlit_frag.hpp"

namespace vkg {
auto DeferredPass::createUnlitPass(Device &device, SceneConfig sceneConfig) -> void {
  GraphicsPipelineMaker maker(device.vkDevice());

  maker.layout(deferredPipeDef.layout())
    .renderPass(*renderPass)
    .subpass(unlitPass)
    .vertexInputAuto(
      {{.stride = sizeof(Vertex::Position),
        .attributes = {{vk::Format::eR32G32B32Sfloat}}},
       {.stride = sizeof(Vertex::Normal), .attributes = {{vk::Format::eR32G32B32Sfloat}}},
       {.stride = sizeof(Vertex::UV), .attributes = {{vk::Format::eR32G32Sfloat}}}})
    .inputAssembly(vk::PrimitiveTopology::eTriangleList)
    .polygonMode(vk::PolygonMode::eFill)
    .cullMode(vk::CullModeFlagBits::eBack)
    .frontFace(vk::FrontFace::eCounterClockwise)
    .depthTestEnable(true)
    .depthWriteEnable(true)
    .depthCompareOp(vk::CompareOp::eLessOrEqual)
    .viewport({})
    .scissor({})
    .dynamicState(vk::DynamicState::eViewport)
    .dynamicState(vk::DynamicState::eScissor);

  maker.blendColorAttachment(false);

  maker
    .shader(
      vk::ShaderStageFlagBits::eVertex,
      Shader{
        shader::deferred::deferred_vert_span, sceneConfig.maxNumTextures,
        sceneConfig.maxNumLights, sceneConfig.numCascades})
    .shader(
      vk::ShaderStageFlagBits::eFragment,
      Shader{
        shader::deferred::unlit_frag_span, sceneConfig.maxNumTextures,
        sceneConfig.maxNumLights, sceneConfig.numCascades});
  unlitTriPipe = maker.createUnique();
  device.name(*unlitTriPipe, "unlit triangle pipeline");

  maker.inputAssembly(vk::PrimitiveTopology::eLineList)
    .polygonMode(vk::PolygonMode::eFill)
    .cullMode(vk::CullModeFlagBits::eNone)
    .lineWidth(lineWidth_);
  unlitLinePipe = maker.createUnique();
  device.name(*unlitLinePipe, "unlit line pipeline");
}
}