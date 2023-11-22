#pragma once
#include "vkg/base/vk_headers.hpp"

namespace vkg {
struct PipelineQuery {

    explicit PipelineQuery(vk::Device device);

    auto reset(vk::CommandBuffer cb) -> void;
    auto begin(vk::CommandBuffer cb) -> void;
    auto end(vk::CommandBuffer cb) -> void;
    auto fetchResults() -> void;

    std::vector<uint64_t> pipelineStats;
    std::vector<std::string> pipelineStatNames;

private:
    vk::Device device;
    vk::UniqueQueryPool queryPool;
};
}
