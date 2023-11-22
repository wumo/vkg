#include "raytracing_pipeline.hpp"
#include "vkg/util/syntactic_sugar.hpp"

namespace vkg {

RayTracingPipelineMaker::RayTracingPipelineMaker(Device &device): device{device} {}
auto RayTracingPipelineMaker::rayGenGroup(Shader &&rayGen) -> RayTracingPipelineMaker & {
    errorIf(!shaders.empty() || hitGroupBegun, "ray gen shader should be the only first shader.");
    addShaderAndGroup(vk::ShaderStageFlagBits::eRaygenNV, std::move(rayGen));
    return *this;
}
auto RayTracingPipelineMaker::missGroup(Shader &&miss) -> RayTracingPipelineMaker & {
    errorIf(hitGroupBegun, "miss shader should be before hitgroup shaders");
    addShaderAndGroup(vk::ShaderStageFlagBits::eMissNV, std::move(miss));
    numMissGroups++;
    return *this;
}
auto RayTracingPipelineMaker::hitGroup(ShaderGroup &&group) -> RayTracingPipelineMaker & {
    hitGroupBegun = true;
    uint32_t closestHit = VK_SHADER_UNUSED_NV;
    uint32_t anyHit = VK_SHADER_UNUSED_NV;
    uint32_t intersection = VK_SHADER_UNUSED_NV;
    if(!group.closestHit.empty()) {
        closestHit = uint32_t(shaders.size());
        shaders.emplace_back(vk::ShaderStageFlagBits::eClosestHitNV, std::move(group.closestHit));
    }
    if(!group.anyHit.empty()) {
        anyHit = uint32_t(shaders.size());
        shaders.emplace_back(vk::ShaderStageFlagBits::eAnyHitNV, std::move(group.anyHit));
    }
    if(!group.intersection.empty()) {
        intersection = uint32_t(shaders.size());
        shaders.emplace_back(vk::ShaderStageFlagBits::eIntersectionNV, std::move(group.intersection));
    }
    vk::RayTracingShaderGroupCreateInfoNV groupInfo{
        intersection == VK_SHADER_UNUSED_NV ? vk::RayTracingShaderGroupTypeNV::eTrianglesHitGroup :
                                              vk::RayTracingShaderGroupTypeNV::eProceduralHitGroup,
        VK_SHADER_UNUSED_NV, closestHit, anyHit, intersection};
    groups.emplace_back(groupInfo);
    return *this;
}
auto RayTracingPipelineMaker::addShaderAndGroup(vk::ShaderStageFlagBits stage, Shader &&shader) -> void {
    auto shaderIndex = uint32_t(shaders.size());
    shaders.emplace_back(stage, std::move(shader));
    vk::RayTracingShaderGroupCreateInfoNV groupInfo{
        vk::RayTracingShaderGroupTypeNV::eGeneral, shaderIndex, VK_SHADER_UNUSED_NV, VK_SHADER_UNUSED_NV,
        VK_SHADER_UNUSED_NV};
    groups.emplace_back(groupInfo);
}
auto RayTracingPipelineMaker::maxRecursionDepth(uint32_t maxRecursionDepth) -> RayTracingPipelineMaker & {
    maxRecursionDepth_ = maxRecursionDepth;
    return *this;
}
auto RayTracingPipelineMaker::createUnique(
    const vk::PipelineLayout &pipelineLayout, const vk::PipelineCache &pipelineCache)
    -> std::tuple<UniqueRayTracingPipeline, ShaderBindingTable> {
    std::vector<vk::PipelineShaderStageCreateInfo> stages;
    std::vector<vk::SpecializationInfo> specializations;
    stages.reserve(shaders.size());
    specializations.reserve(shaders.size());
    for(auto &[stage, shader]: shaders) {
        shader.make(device.vkDevice());
        specializations.emplace_back(shader.specializationInfo());
        stages.push_back({{}, stage, shader.shaderModule(), "main", &specializations.back()});
    }

    vk::RayTracingPipelineCreateInfoNV info{{},
                                            uint32_t(stages.size()),
                                            stages.data(),
                                            uint32_t(groups.size()),
                                            groups.data(),
                                            maxRecursionDepth_,
                                            pipelineLayout};

    vk::UniquePipeline pipeline = device.vkDevice().createRayTracingPipelineNVUnique(pipelineCache, info);

    auto shaderGroupHandleSize = device.rayTracingProperties().shaderGroupHandleSize;
    auto shaderGroupBaseAlignment = device.rayTracingProperties().shaderGroupBaseAlignment;
    auto sbtSize = shaderGroupBaseAlignment * groups.size();
    std::vector<std::byte> shaderHandleStorage(sbtSize);
    device.vkDevice().getRayTracingShaderGroupHandlesNV(
        *pipeline, 0, uint32_t(groups.size()), sbtSize, shaderHandleStorage.data());

    auto sbtBuffer = buffer::hostRayTracingBuffer(device, sbtSize);
    auto *pData = sbtBuffer->ptr<std::byte>();
    for(uint32_t g = 0; g < groups.size(); ++g) {
        memcpy(pData, shaderHandleStorage.data() + g * shaderGroupHandleSize, shaderGroupHandleSize);
        pData += shaderGroupBaseAlignment;
    }
    ShaderBindingTable sbt{
        .shaderBindingTable = std::move(sbtBuffer),
        .rayGenOffset = 0,
        .missGroupOffset = 1 * shaderGroupBaseAlignment,
        .missGroupStride = 1 * shaderGroupBaseAlignment,
        .hitGroupOffset = (1 + numMissGroups) * shaderGroupBaseAlignment,
        .hitGroupStride = shaderGroupBaseAlignment};
    return std::make_tuple(std::move(pipeline), std::move(sbt));
}

}