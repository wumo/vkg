#include "deferred.hpp"
#include "common/quad_vert.hpp"
#include "deferred/lit_frag.hpp"
#include "deferred/lit_atmos_frag.hpp"
#include "deferred/lit_csm_frag.hpp"
#include "deferred/lit_atmos_csm_frag.hpp"

namespace vkg {
auto DeferredPass::createLightingPass(Device &device, SceneConfig sceneConfig) -> void {
    GraphicsPipelineMaker maker(device.vkDevice());

    maker.layout(pipeDef.layout())
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

    maker.shader(vk::ShaderStageFlagBits::eVertex, Shader{shader::common::quad_vert_span})
        .shader(
            vk::ShaderStageFlagBits::eFragment, Shader{shader::deferred::lit_frag_span, sceneConfig.maxNumTextures});
    litPipe = maker.createUnique();
    device.name(*litPipe, "deferred lighting pipeline");

    maker.shader(
        vk::ShaderStageFlagBits::eFragment, Shader{shader::deferred::lit_atmos_frag_span, sceneConfig.maxNumTextures});
    litAtmosPipe = maker.createUnique();
    device.name(*litAtmosPipe, "deferred lighting atmosphere pipeline");

    maker.shader(
        vk::ShaderStageFlagBits::eFragment, Shader{shader::deferred::lit_csm_frag_span, sceneConfig.maxNumTextures});
    litCSMPipe = maker.createUnique();
    device.name(*litCSMPipe, "deferred lighting shadowmap pipeline");

    maker.shader(
        vk::ShaderStageFlagBits::eFragment,
        Shader{shader::deferred::lit_atmos_csm_frag_span, sceneConfig.maxNumTextures});
    litAtmosCSMPipe = maker.createUnique();
    device.name(*litAtmosCSMPipe, "deferred lighting atmosphere shadowmap pipeline");
}
}