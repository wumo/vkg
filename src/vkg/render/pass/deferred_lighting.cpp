#include "deferred.hpp"
#include "common/quad_vert.hpp"
#include "deferred/lighting_frag.hpp"
namespace vkg {
auto DeferredPass::createLightingPass(Device &device, SceneConfig sceneConfig) -> void {
  GraphicsPipelineMaker maker(device.vkDevice());

  maker.layout(deferredPipeDef.layout())
    .renderPass(*renderPass)
    .subpass(litPass)
    .inputAssembly(vk::PrimitiveTopology::eTriangleList)
    .polygonMode(vk::PolygonMode::eFill)
    .cullMode(vk::CullModeFlagBits::eNone)
    .frontFace(vk::FrontFace::eClockwise)
    .depthTestEnable(false)
    .viewport({})
    .scissor({})
    .dynamicState(vk::DynamicState::eViewport)
    .dynamicState(vk::DynamicState::eScissor)
    .dynamicState(vk::DynamicState::eLineWidth);

  maker.blendColorAttachment(false);

  std::span<const uint32_t> lightingSpv;
  lightingSpv = shader::deferred::lighting_frag_span;
  maker.shader(vk::ShaderStageFlagBits::eVertex, Shader{shader::common::quad_vert_span})
    .shader(
      vk::ShaderStageFlagBits::eFragment,
      Shader{
        lightingSpv, sceneConfig.maxNumTextures, sceneConfig.maxNumLights,
        sceneConfig.numCascades});
  litPipe = maker.createUnique();
  device.name(*litPipe, "deferred lighting pipeline");
}
}