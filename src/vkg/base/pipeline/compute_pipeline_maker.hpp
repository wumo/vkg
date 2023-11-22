#pragma once
#include "vkg/base/vk_headers.hpp"
#include <vector>
#include <map>
#include <cstddef>
#include <initializer_list>
#include "pipeline.hpp"
#include "shaders.hpp"

namespace vkg {
class ComputePipelineMaker {
public:
    explicit ComputePipelineMaker(Device &device);
    auto layout(vk::PipelineLayout layout) -> ComputePipelineMaker &;
    auto shader(Shader &&shader) -> ComputePipelineMaker &;
    auto createUnique(vk::PipelineCache pipelineCache = {}) -> vk::UniquePipeline;

private:
    Device &device;
    vk::PipelineLayout layout_;
    Shader shader_;
};
}