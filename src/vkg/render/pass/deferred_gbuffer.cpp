#include "deferred.hpp"
#include "deferred/deferred_vert.hpp"
#include "deferred/gbuffer_frag.hpp"

namespace vkg {
auto DeferredPass::createGbufferPass(Device &device, SceneConfig sceneConfig) -> void {
  GraphicsPipelineMaker maker(device.vkDevice());

  maker.layout(deferredPipeDef.layout())
    .renderPass(*renderPass)
    .subpass(gbPass)
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

  maker.blendColorAttachment(false); //positionAtt
  maker.blendColorAttachment(false); //normalAtt
  maker.blendColorAttachment(false); //albedoAtt
  maker.blendColorAttachment(false); //pbrAtt
  maker.blendColorAttachment(false); //emissiveAtt

  maker
    .shader(
      vk::ShaderStageFlagBits::eVertex,
      Shader{
        shader::deferred::deferred_vert_span, sceneConfig.maxNumTextures,
        sceneConfig.maxNumLights, sceneConfig.numCascades})
    .shader(
      vk::ShaderStageFlagBits::eFragment,
      Shader{
        shader::deferred::gbuffer_frag_span, sceneConfig.maxNumTextures,
        sceneConfig.maxNumLights, sceneConfig.numCascades});
  gbTriPipe = maker.createUnique();
  device.name(*gbTriPipe, "gbuffer triangle pipeline");

  maker.cullMode(vk::CullModeFlagBits::eNone).polygonMode(vk::PolygonMode::eLine);

  gbWireFramePipe = maker.createUnique();
  device.name(*gbWireFramePipe, "gbuffer wireframe pipeline");
}
}