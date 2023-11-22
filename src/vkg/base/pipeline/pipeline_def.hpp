#pragma once
#include "pipeline.hpp"

namespace vkg {
class PipelineLayoutDef {
public:
    auto init(vk::Device device) -> void;
    auto layout() const -> vk::PipelineLayout;
    auto numSets() const -> uint32_t;
    auto layoutDef() const -> const PipelineLayoutMaker &;

protected:
    PipelineLayoutMaker layoutMaker{};
    vk::UniquePipelineLayout pipelineLayout{};
    using vkStage = vk::ShaderStageFlagBits;
};
}
