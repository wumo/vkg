#include "deferred.hpp"
#include "deferred/deferred_vert.hpp"
#include "deferred/transparent_frag.hpp"
#include "common/quad_vert.hpp"
#include "deferred/composite_frag.hpp"

namespace vkg {
auto DeferredPass::createTransparentPass(Device &device, SceneConfig sceneConfig) -> void {
    GraphicsPipelineMaker maker(device.vkDevice());

    maker.layout(pipeDef.layout())
        .renderPass(*renderPass)
        .subpass(transPass)
        .vertexInputAuto(
            {{.stride = sizeof(Vertex::Position), .attributes = {{vk::Format::eR32G32B32Sfloat}}},
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
        .srcColorBlendFactor(vk::BlendFactor::eOne)
        .dstColorBlendFactor(vk::BlendFactor::eOne)
        .colorBlendOp(vk::BlendOp::eAdd)
        .srcAlphaBlendFactor(vk::BlendFactor::eOne)
        .dstAlphaBlendFactor(vk::BlendFactor::eOne)
        .alphaBlendOp(vk::BlendOp::eAdd)
        .colorWriteMask(
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA);

    maker.blendColorAttachment(true)
        .srcColorBlendFactor(vk::BlendFactor::eZero)
        .dstColorBlendFactor(vk::BlendFactor::eOneMinusSrcColor)
        .colorBlendOp(vk::BlendOp::eAdd)
        .srcAlphaBlendFactor(vk::BlendFactor::eZero)
        .dstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
        .alphaBlendOp(vk::BlendOp::eAdd)
        .colorWriteMask(
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA);

    maker
        .shader(
            vk::ShaderStageFlagBits::eVertex, Shader{shader::deferred::deferred_vert_span, sceneConfig.maxNumTextures})
        .shader(
            vk::ShaderStageFlagBits::eFragment,
            Shader{shader::deferred::transparent_frag_span, sceneConfig.maxNumTextures});
    transTriPipe = maker.createUnique();
    device.name(*transTriPipe, "transparent triangle pipeline");

    maker.inputAssembly(vk::PrimitiveTopology::eLineList);
    transLinePipe = maker.createUnique();
    device.name(*transLinePipe, "transparent line pipeline");
}
void DeferredPass::createCompositePass(Device &device, SceneConfig &sceneConfig) {
    GraphicsPipelineMaker maker(device.vkDevice());

    maker.layout(pipeDef.layout())
        .renderPass(*renderPass)
        .subpass(compositePass)
        .inputAssembly(vk::PrimitiveTopology::eTriangleList)
        .polygonMode(vk::PolygonMode::eFill)
        .cullMode(vk::CullModeFlagBits::eNone)
        .frontFace(vk::FrontFace::eClockwise)
        .depthTestEnable(false)
        .viewport({})
        .scissor({})
        .dynamicState(vk::DynamicState::eViewport)
        .dynamicState(vk::DynamicState::eScissor);

    maker.blendColorAttachment(true)
        .srcColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
        .dstColorBlendFactor(vk::BlendFactor::eSrcAlpha)
        .colorBlendOp(vk::BlendOp::eAdd)
        .srcAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
        .dstAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
        .alphaBlendOp(vk::BlendOp::eAdd)
        .colorWriteMask(
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA);

    maker.shader(vk::ShaderStageFlagBits::eVertex, Shader{shader::common::quad_vert_span})
        .shader(
            vk::ShaderStageFlagBits::eFragment,
            Shader{shader::deferred::composite_frag_span, sceneConfig.maxNumTextures});
    compositePipe = maker.createUnique();
    device.name(*compositePipe, "compositePipe");
}
}