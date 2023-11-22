#include "pipeline_query.hpp"

namespace vkg {
PipelineQuery::PipelineQuery(vk::Device device): device{device} {
    using flag = vk::QueryPipelineStatisticFlagBits;

    vk::QueryPoolCreateInfo info{};
    info.pipelineStatistics = flag::eInputAssemblyVertices | flag::eInputAssemblyPrimitives |
                              flag::eVertexShaderInvocations | flag::eGeometryShaderInvocations |
                              flag::eGeometryShaderPrimitives | flag::eClippingInvocations | flag::eClippingPrimitives |
                              flag::eFragmentShaderInvocations | flag::eTessellationControlShaderPatches |
                              flag::eTessellationEvaluationShaderInvocations | flag::eComputeShaderInvocations;
    info.queryType = vk::QueryType ::ePipelineStatistics;
    info.queryCount = 11;
    queryPool = device.createQueryPoolUnique(info);

    pipelineStatNames = {
        vk::to_string(flag::eInputAssemblyVertices),
        vk::to_string(flag::eInputAssemblyPrimitives),
        vk::to_string(flag::eVertexShaderInvocations),
        vk::to_string(flag::eGeometryShaderInvocations),
        vk::to_string(flag::eGeometryShaderPrimitives),
        vk::to_string(flag::eClippingInvocations),
        vk::to_string(flag::eClippingPrimitives),
        vk::to_string(flag::eFragmentShaderInvocations),
        vk::to_string(flag::eTessellationControlShaderPatches),
        vk::to_string(flag::eTessellationEvaluationShaderInvocations),
        vk::to_string(flag::eComputeShaderInvocations),
    };
    pipelineStats.resize(info.queryCount);
}
auto PipelineQuery::reset(vk::CommandBuffer cb) -> void {
    cb.resetQueryPool(*queryPool, 0, uint32_t(pipelineStats.size()));
}
auto PipelineQuery::begin(vk::CommandBuffer cb) -> void { cb.beginQuery(*queryPool, 0, {}); }
auto PipelineQuery::end(vk::CommandBuffer cb) -> void { cb.endQuery(*queryPool, 0); }
auto PipelineQuery::fetchResults() -> void {
    device.getQueryPoolResults(
        *queryPool, 0, 1, pipelineStats.size() * sizeof(uint64_t), pipelineStats.data(), sizeof(uint64_t),
        vk::QueryResultFlagBits::e64);
}

}