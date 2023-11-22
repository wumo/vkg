#pragma once
#include "vkg/base/resource/buffers.hpp"
#include <vector>
#include <map>
#include <cstddef>
#include <initializer_list>
#include "pipeline.hpp"
#include "shaders.hpp"

namespace vkg {
using UniqueRayTracingPipeline = vk::UniqueHandle<vk::Pipeline, vk::DispatchLoaderDynamic>;

struct ShaderBindingTable {
    std::unique_ptr<Buffer> shaderBindingTable;
    vk::DeviceSize rayGenOffset, missGroupOffset, missGroupStride, hitGroupOffset, hitGroupStride;
};

struct ShaderGroup {
    Shader closestHit;
    Shader intersection;
    Shader anyHit;
};

class RayTracingPipelineMaker: SpecializationMaker {
public:
    explicit RayTracingPipelineMaker(Device &device);
    auto rayGenGroup(Shader &&rayGen) -> RayTracingPipelineMaker &;
    auto missGroup(Shader &&miss) -> RayTracingPipelineMaker &;
    auto hitGroup(ShaderGroup &&group = {}) -> RayTracingPipelineMaker &;

    auto maxRecursionDepth(uint32_t maxRecursionDepth) -> RayTracingPipelineMaker &;
    auto createUnique(const vk::PipelineLayout &pipelineLayout, const vk::PipelineCache &pipelineCache = nullptr)
        -> std::tuple<UniqueRayTracingPipeline, ShaderBindingTable>;

private:
    auto addShaderAndGroup(vk::ShaderStageFlagBits stage, Shader &&shader) -> void;

    struct ShaderInfo {
        vk::UniqueShaderModule shaderModule;
        const vk::SpecializationInfo *pSpecializationInfo;
        int32_t specializationIndex{-1};
    };

    Device &device;

    std::vector<std::pair<vk::ShaderStageFlagBits, Shader>> shaders;
    std::vector<vk::RayTracingShaderGroupCreateInfoNV> groups;
    uint32_t numMissGroups{0};
    uint32_t maxRecursionDepth_{1};
    bool hitGroupBegun{false};
};
}