#pragma once
#include "vkg/base/base.hpp"
#include "vkg/render/scene_config.hpp"
#include "vkg/render/graph/frame_graph.hpp"
#include "vkg/math/glm_common.hpp"

namespace vkg {
struct ComputeTransfPassIn {
    FrameGraphResource<BufferInfo> transforms;
    FrameGraphResource<BufferInfo> meshInstances;
    FrameGraphResource<uint32_t> meshInstancesCount;
    FrameGraphResource<SceneConfig> sceneConfig;
};
struct ComputeTransfPassOut {
    FrameGraphResource<BufferInfo> matrices;
};

class ComputeTransf: public Pass<ComputeTransfPassIn, ComputeTransfPassOut> {
public:
    void setup(PassBuilder &builder) override;
    void compile(RenderContext &ctx, Resources &resources) override;
    void execute(RenderContext &ctx, Resources &resources) override;

private:
    struct PushConstant {
        uint32_t totalMeshInstances;
        uint32_t frame;
    } pushConstant{};
    struct ComputeTransfSetDef: DescriptorSetDef {
        __buffer__(meshInstances, vkStage::eCompute);
        __buffer__(transforms, vkStage::eCompute);
        __buffer__(matrices, vkStage::eCompute);
    } setDef;
    struct ComputeTransfPipeDef: PipelineLayoutDef {
        __push_constant__(constant, vkStage::eCompute, PushConstant);
        __set__(transf, ComputeTransfSetDef);
    } pipeDef;
    vk::UniquePipeline pipe;
    const uint32_t local_size = 64;

    vk::UniqueDescriptorPool descriptorPool;

    struct FrameResource {
        std::unique_ptr<Buffer> matrices;
        vk::DescriptorSet set;
    };
    std::vector<FrameResource> frames;
    bool init{false};
};

}
